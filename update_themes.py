import json
import os

theme_files = ["resources/themes/default_dark.json", "resources/themes/default_dark_neon.json"]

cpp_rules = {
    "language_name": "C++",
    "file_extensions": [".cpp", ".hpp", ".cxx", ".hxx", ".cc", ".hh", ".c", ".h"],
    "highlighting_rules": [
        {"type": "keywords", "list": ["alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_commit", "atomic_noexcept", "auto", "bitand", "bitor", "bool", "break", "case", "catch", "char", "char8_t", "char16_t", "char32_t", "class", "compl", "concept", "const", "consteval", "constexpr", "constinit", "const_cast", "continue", "co_await", "co_return", "co_yield", "decltype", "default", "delete", "do", "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false", "float", "for", "friend", "goto", "if", "inline", "int", "long", "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private", "protected", "public", "reflexpr", "register", "reinterpret_cast", "requires", "return", "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct", "switch", "synchronized", "template", "this", "thread_local", "throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using", "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"], "style_key": "keyword"},
        {"type": "line_comment", "start_delimiter": "//", "style_key": "comment"},
        {"type": "multi_line_string", "name": "Block Comment", "start_pattern": "/\\*", "end_pattern": "\\*/", "style_key": "comment"},
        {"type": "multi_line_string", "name": "String", "start_pattern": "\"", "end_pattern": "\"", "style_key": "string"},
        {"type": "pattern", "name": "Character", "pattern": "'([^']|\\\\.)'", "style_key": "string"},
        {"type": "pattern", "name": "Preprocessor", "pattern": "^\\s*#.*", "style_key": "preprocessor"},
        {"type": "pattern", "name": "Number", "pattern": "\\\\b([0-9]+\\\\.?[0-9]*|\\\\.[0-9]+)([eE][-+]?[0-9]+)?f?\\\\b|\\\\b0[xX][0-9a-fA-F]+L?U?\\\\b|\\\\b[0-9]+L?U?\\\\b", "style_key": "number"},
        {"type": "keywords", "name": "Common Types", "list": ["std::string", "std::vector", "std::map", "std::list", "std::cout", "std::cin", "std::endl", "size_t", "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t"], "style_key": "type_name"}
    ]
}

java_rules = {
    "language_name": "Java",
    "file_extensions": [".java"],
    "highlighting_rules": [
        {"type": "keywords", "list": ["abstract", "assert", "boolean", "break", "byte", "case", "catch", "char", "class", "const", "continue", "default", "do", "double", "else", "enum", "extends", "final", "finally", "float", "for", "goto", "if", "implements", "import", "instanceof", "int", "interface", "long", "native", "new", "package", "private", "protected", "public", "return", "short", "static", "strictfp", "super", "switch", "synchronized", "this", "throw", "throws", "transient", "try", "void", "volatile", "while", "true", "false", "null"], "style_key": "keyword"},
        {"type": "line_comment", "start_delimiter": "//", "style_key": "comment"},
        {"type": "multi_line_string", "name": "Block Comment", "start_pattern": "/\\*", "end_pattern": "\\*/", "style_key": "comment"},
        {"type": "multi_line_string", "name": "String", "start_pattern": "\"", "end_pattern": "\"", "style_key": "string"},
        {"type": "pattern", "name": "Character", "pattern": "'([^']|\\\\.)'", "style_key": "string"},
        {"type": "pattern", "name": "Annotations", "pattern": "@[A-Za-z_][A-Za-z0-9_]*", "style_key": "preprocessor"},
        {"type": "pattern", "name": "Number", "pattern": "\\\\b([0-9]+\\\\.?[0-9]*|\\\\.[0-9]+)([eE][-+]?[0-9]+)?([fFdDlL])?\\\\b|\\\\b0[xX][0-9a-fA-F]+L?\\\\b|\\\\b[0-9]+L?\\\\b", "style_key": "number"},
        {"type": "keywords", "name": "Common Types", "list": ["String", "Integer", "Double", "Boolean", "List", "ArrayList", "Map", "HashMap", "Object", "Byte", "Short", "Long", "Float", "Character", "Void"], "style_key": "type_name"}
    ]
}

javascript_rules = {
    "language_name": "JavaScript",
    "file_extensions": [".js", ".mjs", ".cjs"],
    "highlighting_rules": [
        {"type": "keywords", "list": ["abstract", "arguments", "await", "boolean", "break", "byte", "case", "catch", "char", "class", "const", "continue", "debugger", "default", "delete", "do", "double", "else", "enum", "eval", "export", "extends", "false", "final", "finally", "float", "for", "function", "goto", "if", "implements", "import", "in", "instanceof", "int", "interface", "let", "long", "native", "new", "null", "package", "private", "protected", "public", "return", "short", "static", "super", "switch", "synchronized", "this", "throw", "throws", "transient", "true", "try", "typeof", "var", "void", "volatile", "while", "with", "yield", "async", "of", "get", "set"], "style_key": "keyword"},
        {"type": "line_comment", "start_delimiter": "//", "style_key": "comment"},
        {"type": "multi_line_string", "name": "Block Comment", "start_pattern": "/\\*", "end_pattern": "\\*/", "style_key": "comment"},
        {"type": "multi_line_string", "name": "Double Quoted String", "start_pattern": "\"", "end_pattern": "\"", "style_key": "string"},
        {"type": "multi_line_string", "name": "Single Quoted String", "start_pattern": "'", "end_pattern": "'", "style_key": "string"},
        {"type": "multi_line_string", "name": "Template Literal", "start_pattern": "`", "end_pattern": "`", "style_key": "string"},
        {"type": "pattern", "name": "Regular Expression", "pattern": "/[^/\\n\\\\]*(?:\\\\[^\\n]|[^/\\n\\\\])*/[gimyus]*", "style_key": "preprocessor"},
        {"type": "pattern", "name": "Number", "pattern": "\\\\b([0-9]+\\\\.?[0-9]*|\\\\.[0-9]+)([eE][-+]?[0-9]+)?\\\\b|\\\\b0[xX][0-9a-fA-F]+\\\\b|\\\\b0[bB][01]+\\\\b|\\\\b0[oO][0-7]+\\\\b|\\\\b[0-9]+\\\\b", "style_key": "number"},
        {"type": "keywords", "name": "Common Objects/Functions", "list": ["console", "log", "Math", "JSON", "Promise", "Array", "Object", "String", "Number", "Boolean", "Function", "Symbol", "Date", "RegExp", "Error", "Map", "Set", "WeakMap", "WeakSet", "document", "window", "require", "module", "exports"], "style_key": "type_name"}
    ]
}

rules_to_add = {
    "cpp": cpp_rules,
    "java": java_rules,
    "javascript": javascript_rules
}

for theme_file_path in theme_files:
    theme_dir = os.path.dirname(theme_file_path)
    if not os.path.exists(theme_dir):
        print(f"Theme directory {theme_dir} does not exist. Skipping {theme_file_path}.")
        continue

    try:
        if not os.path.exists(theme_file_path):
            print(f"Theme file {theme_file_path} does not exist. Initializing with default structure.")
            theme_dict = {
                "name": "Default Theme",
                "type": "dark",
                "colors": {},
                "syntax_highlighting": {}
            }
        else:
            with open(theme_file_path, 'r') as f:
                theme_dict = json.load(f)
    except Exception as e:
        print(f"Error reading or parsing {theme_file_path}: {e}")
        continue

    syntax_rules_dict = theme_dict.setdefault("syntax_highlighting", {})

    for lang_key, lang_rules in rules_to_add.items():
        syntax_rules_dict[lang_key] = lang_rules
        print(f"Added/Updated {lang_key} syntax rules in {theme_file_path}")

    try:
        with open(theme_file_path, 'w') as f:
            json.dump(theme_dict, f, indent=4)
        print(f"Successfully updated and wrote {theme_file_path}")
    except Exception as e:
        print(f"Error writing {theme_file_path}: {e}")
