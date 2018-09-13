#!/usr/bin/env dart
import "dart:io";
import "dart:convert";
import "package:path/path.dart" as pathpkg;
import "package:args/args.dart";
import "package:yaml/yaml.dart";

void fill(List<bool> bits, String lo, String hi) {
  int ilo = lo.codeUnitAt(0);
  int ihi = hi.codeUnitAt(0);
  for (int i = ilo; i <= ihi; i++) {
    bits[i] = true;
  }
}

List<bool> AlphaNum = () {
  var bits = new List<bool>.filled(256, false);
  fill(bits, "A", "Z");
  fill(bits, "a", "z");
  fill(bits, "0", "9");
  fill(bits, "_", "_");
  return bits;
}();

bool isAlphaNum(String ch) {
  return AlphaNum[ch.codeUnitAt(0)];
}

final RegExp rxPreProc = new RegExp(r"\s*#\s*(\w+)");
final RegExp rxNoWard = new RegExp(r"\s*#\s*ifndef\s+WARD_ENABLED");
final RegExp rxAllCaps = new RegExp(r"^[A-Z_]+$");

enum TokenType {
  WhiteSpace,
  NewLine,
  Comment,
  StringLiteral,
  Identifier,
  Operator,
  Unknown,
}

const tokWhiteSpace = TokenType.WhiteSpace;
const tokNewLine = TokenType.NewLine;
const tokStringLiteral = TokenType.StringLiteral;
const tokComment = TokenType.Comment;
const tokIdentifier = TokenType.Identifier;
const tokOperator = TokenType.Operator;
const tokUnknown = TokenType.Unknown;

class Token {
  TokenType type;
  String text;
  String aux;
  Token(this.type, this.text, {this.aux});
}

class FileParser {
  final String path;
  List<String> lines;
  Set<String> names;
  bool inString = false;
  bool inComment = false;
  bool transform = false;
  int lineNo = 0;
  int noward = 0;
  StringBuffer out = null;

  FileParser(this.path) {
    var file = new File(path);
    lines = file.readAsLinesSync(encoding: latin1);
  }

  void scanLine() {
    var line = lines[lineNo];
    out = null;
    lineNo++;
    int p = 0;
    int oldp = 0;
    if (!inString && !inComment) {
      var match = rxPreProc.firstMatch(line);
      if (match != null) {
        switch (match[1]) {
          case "ifndef":
          case "if":
          case "ifdef":
            if (rxNoWard.hasMatch(line)) {
              noward = 1;
            } else if (noward > 0) {
              noward++;
            }
            break;
          case "elif":
            if (noward == 1) noward = 0;
            break;
          case "else":
            if (noward == 1) noward = 0;
            break;
          case "endif":
            if (noward > 0) noward--;
            break;
        }
        return;
      }
    }
    while (p < line.length) {
      String ch = line[p++];
      if (inString) {
        if (ch == "\"") {
          inString = false;
        } else if (ch == "\\") {
          p++;
        }
      } else if (inComment) {
        if (ch == "*" && p < line.length && line[p] == "/") {
          p++;
          inComment = false;
        }
      } else {
        switch (ch) {
          case "\"":
            inString = true;
            break;
          case "/":
            if (p < line.length && line[p] == '*') {
              p++;
              inComment = true;
            } else if (p < line.length && line[p] == '/') {
              // skip to end of line
              p = line.length;
            }
            break;
          default:
            if (isAlphaNum(ch)) {
              if (transform) {
                var idbuf = new StringBuffer(ch);
                var idstart = p - 1;
                while (p < line.length && isAlphaNum(line[p])) {
                  idbuf.write(line[p++]);
                }
                var id = idbuf.toString();
                if (noward > 0 && names.contains(id)) {
                  if (out == null) out = new StringBuffer();
                  out.write(line.substring(oldp, idstart));
                  oldp = p;
                  out.write("UNSAFE_");
                  out.write(id);
                }
              } else {
                var idbuf = new StringBuffer(ch);
                while (p < line.length && isAlphaNum(line[p])) {
                  idbuf.write(line[p++]);
                }
                var id = idbuf.toString();
                while (p < line.length) {
                  var ch = line[p];
                  if (ch == '(') {
                    if (noward > 0 && !id.startsWith("UNSAFE_")) {
                      names.add(id);
                    }
                    break;
                  }
                  if (ch != ' ' && ch != '\t') {
                    break;
                  }
                  p++;
                }
              }
            }
        }
      }
    }
    if (out != null) out.write(line.substring(oldp, line.length));
  }

  void scanFile() {
    lineNo = 0;
    noward = 0;
    inString = false;
    inComment = false;
    names = new Set<String>();
    transform = false;
    while (lineNo < lines.length) {
      scanLine();
    }
  }

  bool transformFile(Set<String> ids) {
    bool changed = false;
    lineNo = 0;
    noward = 0;
    inString = false;
    inComment = false;
    names = ids;
    transform = true;
    while (lineNo < lines.length) {
      scanLine();
      if (out != null) {
        lines[lineNo - 1] = out.toString();
        changed = true;
      }
    }
    return changed;
  }

  Set<String> allCapsNames() =>
      names.where((s) => rxAllCaps.hasMatch(s)).toSet();
}

void fatal(String message) {
  print(message);
  exit(1);
}

List<String> allFiles(List<String> paths) {
  var result = <String>[];
  for (final path in paths) {
    if (FileSystemEntity.isDirectorySync(path)) {
      result.addAll(new Directory(path)
          .listSync(recursive: true)
          .where((fse) => fse is File)
          .map((fse) => fse.path)
          .where((path) => path.endsWith(".c") || path.endsWith(".h")));
    } else {
      result.add(path);
    }
  }
  return result;
}

String programName() =>
    pathpkg.basenameWithoutExtension(Platform.script.toString());

ArgResults parseArgs(ArgParser parser, List<String> args) {
  try {
    return parser.parse(args);
  } catch (e) {
    fatal("${programName()}: ${e.message}"); // doesn't return
    return null; // make dartanalyzer happy
  }
}

void help(ArgParser argparser) {
  String indent(String str) =>
      str.replaceAll(new RegExp("^", multiLine: true), "  ");
  print("Usage: ${programName()} [OPTIONS] [PATHS]\n");
  print(indent(argparser.usage));
  exit(0);
}

ArgParser makeArgParser() {
  var argparser = new ArgParser();
  argparser.addFlag("scan", abbr: "S", help: "scan files and directories");
  argparser.addOption("rewrite-from",
      abbr: "R", help: "rewrite files and directories", valueHelp: "IDFILE");
  argparser.addOption("resume", help: "resume scan from this identifiers",
      abbr: "r", valueHelp: "IDFILE");
  argparser.addFlag("help",
      abbr: "h", negatable: false, help: "show this help");
  return argparser;
}

Map<String, List<String>> loadIds(String filename) {
  var yaml = loadYaml(new File(filename).readAsStringSync());
  List<T> copy<T>(List<T> list) => list..addAll([]);
  return {
    "unsafe" : copy(yaml["unsafe"] ?? []),
    "skip" : copy(yaml["skip"] ?? []),
  };
}

void main(List<String> args) {
  ArgParser argparser = makeArgParser();
  var opts = parseArgs(argparser, args);
  if (opts["help"]) {
    help(argparser);
  }
  if (opts.wasParsed("scan") == opts.wasParsed("rewrite-from")) {
    fatal("specify exactly one of --scan and --rewrite-from");
  }

  List<String> files = allFiles(opts.rest);

  if (opts["scan"]) {
    var ids = opts["resume"] != null ? loadIds(opts["resume"]) :
      { "unsafe": [], "skip": [] };
    var skip = ids["skip"].toSet();
    for (final filename in files) {
      final parser = new FileParser(filename);
      parser.scanFile();
      ids["unsafe"].addAll(parser.allCapsNames());
      ids["unsafe"] = ids["unsafe"]
          .toSet()
	  .difference(skip)
	  .toList();
    }
    print("unsafe:");
    for (var id in ids["unsafe"]..sort()) {
      print("- $id");
    }
    if (ids["skip"].length != 0) {
      print("skip:");
      for (var id in ids["skip"]..sort()) {
	print("- $id");
      }
    }
  } else {
    // rewrite
    var ids = loadIds(opts["rewrite-from"])["unsafe"].toSet();
    for (final filename in files) {
      final parser = new FileParser(filename);
      if (parser.transformFile(ids)) {
        var file = new File(filename + ".tmp");
        var output = parser.lines.join("\n") + "\n";
        file.writeAsStringSync(output);
        file.renameSync(filename);
      }
    }
  }
}
