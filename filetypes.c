/** 
        filetypes.c
        
*/

#include <stddef.h>
#include "const.h"
#include "syntax.h"
#include "highlight.h"

char *C_HL_extensions[] = { ".c", ".h", ".cpp", NULL }; 
char *C_HL_keywords[] = {
	"auto", "break", "case", "const", "continue", "default", "do", 
	"else", "enum", "extern", "for", "goto", "if", "register", 
	"return", "signed", "sizeof", "static", "struct", "switch",
	"typedef", "union", "unsigned", "volatile", "while",

	/* C99 */
	"restrict", "inline", "_Bool", "bool", "_Complex", "complex", 
	"_Imaginary", "imaginary", "_Pragma", 
	/* C11 */
	"_Alignas", "alignas", "_Alignof", "alignof", 
	"_Atomic", "atomic_bool", "atomic_int", 
	"_Generic", "_Noreturn", "noreturn", 
	"_Static_assert", "static_assert", 
	"_Thread_local", "thread_local",  /* C11 */

	/* "asm", "fortran", */

	/* Those C++(upto 11) keywords not in C(upto11). */ 
	"and", "and_eq", "asm", "atomic_cancel", "atomic_commit", 
	"atomic_noexcept", "bitand", "bitor", "catch", "char16_t", 
	"char32_t", "class", "compl", "concept", "constexpr", 
	"const_cast", "decltype", "delete", "dynamic_cast", "explicit", 
	"export", "false", "friend", "import", "module", "mutable", 
	"namespace", "new", "noexcept", "not", "not_eq", "nullptr", 
	"operator", "or", "or_eq", "private", "protected", "public",
	"register", "reinterpret_cast", "requires", "static_assert",
	"static_cast", "synchronized", "template", "this", "throw",
	"true", "try", "typeid", "typename", "using", "virtual",
	"wchar_t", "xor", "xor_eq",  /* C++ 11 */

	/* cpp */
	"#if", "#ifdef", "#ifndef",
	"#elif", "#else", "#endif",  
	"#define", "#defined", "#undef", 
	"#include", "#pragma", "#line", "#error",

	/* C types */
	"int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|", "void|", 
	NULL
};

char *Java_HL_extensions[] = { ".java", NULL };
char *Java_HL_keywords[] = {
	"abstract", "assert", "break", "case", "catch", "class", "const", 
	"continue", "default", "do", "else", "enum", "extends", "final",
	"finally", "for", "goto", "if", "implements", "import", "instanceof", 
	"native", "new", "package", "private", "protected", "public", "return",  
	"static", "strictfp", "super", "switch", "synchronized", "this", "throw",
	"transient", "try", "void", "volatile", "while", "false", "null", "true",

	"boolean|", "byte|", "char|", "double|", "float|", "int|", "long|", 
	"Boolean|", "Byte|", "Character|", "Double|", "Float|", "Integer|", "Long|",
	"short|", "Short|",

	"@Deprecated", "@Override", "@SuppressWarnings", "@SafeVarargs", 
	"@FunctionalInterface", "@Retention", "@Documented", "@Target", 
	"@Inherited", "@Repeatable",

	"@Test", // There will be more.

	NULL
};

char *Python_HL_extensions[] = { ".py", NULL };
char *Python_HL_keywords[] = {
	"False", "None", "True", 
	"and", "as", "assert", "break", "class", "continue", "def", "del", 
	"elif", "else", "except", "finally", "for", "from", "global", 
	"if", "import", "in", "is", "lambda", "nonlocal", "not", "or",
	"pass", "raise", "return", "try", "while", "with", "yield",

	"int|", "float|", "complex|", "decimal|", "fraction|", 
	"container|", "iterator|", "list|", "tuple|", "range|", 
	"bytes|", "bytearray|", 
	"set|", "frozenset|", 
	"dict|", 

	NULL
};

char *Text_HL_extensions[] = { ".txt", ".ini", ".cfg", NULL };
char *Text_HL_keywords[] = { NULL };

char *Makefile_HL_extensions[] = { "Makefile", "makefile", ".mk", ".mmk", NULL };
char *Makefile_HL_keywords[] = { NULL };

char *Erlang_HL_extensions[] = { ".erl", NULL };
char *Erlang_HL_keywords[] = {
        "after", "and", "andalso", "band", "begin", "bnot", "bor", 
        "bsl", "bsr", "bxor", "case", "catch", "cond", "div", "end",
        "fun", "if", "let", "not", "of", "or", "orelse", "receive",
        "rem", "try", "when", "xor", 
        "->", // can do? 
        "-module", "-export", "-import",
        NULL
};

char *JS_HL_extensions[] = { ".js", NULL };
char *JS_HL_keywords[] = {
        "abstract", "arguments", "await", 
        "boolean|", "break", "byte|",
        "case", "catch ", "char|", "class", "const", "continue",
        "debugger", "default", "delete", "do", "double|", 
        "else", "enum", "eval", "export", "extends", 
        "false", "final", "finally", "float|", "for", "function", 
        "goto",
        "if", "implements", "import", "in", "instanceof", "int|", "interface", 
        "let", "long|",
        "native", "new", "null", 
        "package", "private", "protected", "public",
        "return", 
        "short|", "static", "super", "switch", "synchronized", 
        "this", "throw", "throws", "transient", "true", "try", "typeof", 
        "var", "void|", "volatile",
        "while", "with", 
        "yield", 

        /* And even more: JavaScript Object, Properties, and Methods. */
        "Array|", "Date|", "hasOwnProperty", 
        "Infinity|", "isFinite", "isNaN", "isPrototypeOf",
        "length", "Math", "NaN|", "name", "Number|", "Object|",
        "prototype", "String|", "toString", "undefined", "valueOf",
        
        /* Java reserved words. */ 
        
        "getClass", "java", "JavaArray|", "javaClass", "JavaClass", 
        "JavaObject|", "JavaPackage",
        
        /* Other reserved words. HTML and Window objects and properties. */

        "alert", "all", "anchor", "anchors", "area", "assign", 
        "blur", "button", 
        "checkbox", "clearInterval", "clearTimeout", "clientInformation",
        "close", "closed", "confirm", "constructor", "crypto",
        "decodeURI", "decodeURIComponent", "defaultStatus", "document",
        "element", "elements", "embed", "embeds", "encodeURI", 
        "encodeURIComponent", "escape", "event", 
        "fileUpload", "focus", "form", "forms", "frame", "frames", "frameRate",
        "hidden", "history", 
        "image", "images", "innerHeight", "innerWidth", 
        "layer", "layers", "link", "location", 
        "mimeTypes", 
        "navigate", "navigator", 
        "offscreenBuffering", "open", "opener", "option", "outerHeight", 
        "outerWidth", 
        "packages", "pageXOffset", "pageYOffset", "parent", "parseFloat", 
        "parseInt", "password", "pkcs11", "plugin", "prompt", "propertyIsEnum",
        "radio", "reset", 
        "screenX", "screenY", "scroll", "secure", "select", "self", 
        "setInterval", "setTimeout", "status", "submit", 
        "taint", "text", "textarea", "top", "unescape", "untaint", 
        "window",

        /* HTML Event Handlers. */
        
        "onblur", "onclick", "onerror", "onfocus", "onkeydown", "onkeyup", 
        "onmouseover", "onmouseup", "onmousedown", "onload", "onsubmit",
                
        NULL
};

char *Shell_HL_extensions[] = { ".sh", NULL };
char *Shell_HL_keywords[] = {
	// compgen -k
	"if", "then", "else", "elif", "fi",
	"case", "esac", "for", "select", "while", "until",
	"do", "done", "in", "function", "time", "[[", "]]",
	NULL
};

char *Perl_HL_extensions[] = { ".pl", ".perl", NULL };
char *Perl_HL_keywords[] = {
	/* Perl functions. */
	"-A", "-B", "-b", "-C", "-c", "-d", "-e", "-f", "-g", "-k", "-l", "-M", "-O", "-o", 
	"-p", "-r" "-R", "-S", "-s", "-T", "-t", "-u", "-w", "-W", "-X", "-x", "-z",

	"abs", "accept", "alarm", "atan2", "AUTOLOAD", 
	"BEGIN", "bind", "binmode", "bless", "break", 
	"caller", "chdir", "CHECK", "chmod", "chomp", "chop", "chown", "chr", "chroot",
	"close", "closedir", "connect", "cos", "crypt", 
	"dbmclose", "defined", "delete", "DESTROY", "die", "dump", 
	"each", "END", "endgrent", "endhostent", "endnetent", "endprotoent", "endpwent",
	"endservent", "eof", "eval", "exec", "exists", "exit", 
	"fcntl", "fileno", "flock", "fork", "format", "formline", 
	"getc", "getgrent", "getgrgid", "getgrnam", "gethostbyaddr", "gethostbyname",
	"gethostent", "getlogin", "getnetbyaddr", "getnetbyname", "getnetent", 
	"getpeername", "getpgrp", "getppid", "getpriority", "getprotobyname", 
	"getprotobynumber", "getprotoent", "getpwent", "getpwnam", "getpwuid", 
	"getservbyport", "getservent", "getsockname", "getsockopt", "glob", "gmtime", 
	"goto", "grep", 
	"hex",
	"index",
	"INIT", "int", "ioctl", 
	"join",
	"keys", "kill",
	"last", "lc", "lcfirst", "length", "link", "listen", "local", "localtime", 
	"log", "lstat",  
	"map", "mkdir", "msgctl", "msgget", "msgrcv", "msgsnd", "my",
	"next", "not",
	"oct", "open", "opendir", "ord", "our", 
	"pack", "pipe", "pop", "pos", "print", "printf", "prototype", "push",
	"quotemeta", 
	"rand", "read", "readdir", "readline", "readlink", "readpipe", "recv",
	"redo", "ref", "rename", "require", "reset", "return", "reverse", "rewinddir", 
	"rindex", "rmdir", 
	"say", "scalar", "seek", "seekdir", "select", "semctl", "semget", "semop", 
	"send", "setgrent", "sethostent", "setnetent", "setpgrp", "setpriority", 
	"setprotoent", "setpwent", "setservent", "setsockopt", "shift", 
	"shmctl", "shmget", "shmread", "shmwrite", "shutdown", "sin", "sleep",
	"socket", "socketpair", "sort", "splice", "split", "sprintf", "sqrt", "srand",
	"stat", "state", "study", "substr", "symlink", "syscall", "sysopen", "sysread",
	"sysseek", "system", "syswrite", 
	"tell", "telldir", "tie", "tied", "time", "times", "truncate", 
	"uc", "ucfirst", "umask", "undef", "UNITCHECK", "unlink", "unpack", "unshift",
	"untie", "use", "utime",
	"values", "vec", "wait", "waitpid", "wantarray", "warn", "write",

	/* Perl syntax */

	"__DATA__", "__END__", "__FILE__", "__LINE__", "__PACKAGE__", 
	"and", "cmp", "continue", "CORE", "do", "else", "elsif", "eq", "exp", 
	"for", "foreach", "ge", "gt", "if", "le", "lock", "lt", 
	"m", "ne", "no", "or", "package", "q", "qq", "qr", "qw", "qx", 
	"s", "sub", "tr", "unless", "until", "while", "xor", "y",

	NULL
};

char *Ruby_HL_extensions[] = { ".rb", "Vagrantfile", NULL };
char *Ruby_HL_keywords[] = {
        "__ENCODING__", "__LINE__", "__FILE__", "BEGIN", "END", 
        "alias", "and", "begin", "break", 
        "case", "class", 
        "def", "defined?", "do", 
        "else", "elsif", "end", "ensure", 
        "false", "for",
        "if", "in", 
        "module", 
        "next", "nil", "not",
        "or", 
        "redo", "rescue", "retry", "return", 
        "self", "super", 
        "then", "true", 
        "undef", "unless", "until", 
        "when", "while",
        "yield", 
        
   
    NULL
}; 

/* https://guide.elm-lang.org */
char *Elm_HL_extensions[] = { ".elm", NULL };
char *Elm_HL_keywords[] = {
        "if", "then", "else", "case", "of", "let", "in", "type", 
        /* Maybe not 'where' */
        "module", "where", "import", "exposing", "as", "port",
        "infix", "infixl", "infixr", 
        
        NULL
};  

char *PHP_HL_extensions[] = { ".php", NULL };
char *PHP_HL_keywords[] = {
        "__halt_compiler", 
        "abstract", "and", "array", "as", 
        "break", 
        "callable", "case", "catch", "class", "clone", "const", "continue",
        "declare", "default", "die", "do", 
        "echo", "else", "elseif", "empty", "enddeclare", "endfor", 
        "endforeach", "endif", "endswitch", "endwhile", "eval", "exit", 
        "extends", 
        "final", "finally", "for", "foreach", "function", 
        "global", "goto", 
        "if", "implements", "include", "include_once", "instanceof",
        "insteadof", "interface", "isset", 
        "list", 
        "namespace", "new", 
        "or",
        "print", "private", "protected", "public", 
        "require", "require_once", "return", 
        "static", "switch", 
        "throw", "trait", "try", 
        "unset", "use", 
        "var", 
        "while", 
        "xor",
        "yield",
        
        "__CLASS__", "__DIR__", "__FILE__", "__FUNCTION__", "__LINE__", 
        "__METHOD__", "__NAMESPACE__", "__TRAIT__",
        
        NULL
};

/* https://docs.bazel.build/versions/master/be/overview.html */
char *Bazel_HL_extensions[] = { "WORKSPACE", "BUILD", ".bzl", NULL };
char *Bazel_HL_keywords[] = { 
        // Functions
        "load", "package", "package_group", "licenses", "exports_files",
        "glob", "select", "workspace",
        // Android
        "android_binary", "android_library", "aar_import", 
        "android_device", "android_ndk_repository", 
        "android_sdk_repository", 
        // C/C++
        "cc_binary", "cc_inc_library", "cc_library", "cc_proto_library",
        "cc_test",  
        // Java
        "java_binary", "java_import", "java_library",
        "java_lite_proto_library", "java_proto_library", "java_test", 
        "java_plugin", "java_runtime", "java_runtime_suite", 
        "java_toolchain", 
        // Objective_C
        "apple_binary", "apple_static_library", "apple_stub_library",
        "ios_application", "ios_extension", "ios_extension_binary", 
        "objc_binary", "j2objc_library", "objc_bundle", 
        "objc_bundle_library", "objc_framework", "objc_import",
        "objc_library", "objc_proto_library", "ios_test",
        // Protocol Buffer
        "proto_lang_toolchain", "proto_library",
        //Python
        "py_binary", "py_library", "py_test", "py_runtime", 
        // Shell
        "sh_binary", "sh_library", "sh_test", 
        // Extra Actions
        "action_listener", "extra_action", 
        // General
        "filegroup", "genquery", "test_suite", "alias",
        "config_setting", "genrule",
        // Platform
        "constraint_setting", "contraint_value", "platform", "toolchain", 
        // Workspace
        "bind", "git_repository", "http_archive", "http_file", "http_jar",
        "local_repository", "maven_jar", "maven_server", 
        "new_git_repository", "new_http_archive", "new_local_repository",
        "xcode_config", "xcode_version",
        NULL
};

/* A .dockerignore file does not contain any Dockerfile keywords (unless by chance), 
   it's a newline separated list of patterns similar to file globs. But I wanted to 
   add it here to empasize its 'Dockerness'. */ 
char *Dockerfile_HL_extensions[] = { "Dockerfile", ".dockerignore", NULL };
char *Dockerfile_HL_keywords[] = {
        "ADD", "ARG", 
        "CMD", "COPY", 
        "ENTRYPOINT", "ENV", "EXPOSE", 
        "FROM",
        "HEALTHCHECK", 
        "LABEL", 
        "MAINTAINER", /* Deprecated */
        "ONBUILD", 
        "RUN",
        "SHELL", "STOPSIGNAL", 
        "USER", 
        "VOLUME", 
        "WORKDIR", 
        NULL
};         

char *SQL_HL_extensions[] = { ".sql", ".SQL", NULL };
char *SQL_HL_keywords[] = {
        /* https://www.drupal.org/docs/develop/coding-standards/list-of-sql-reserved-words */
        "A", "ABORT", "ABS", "ABSOLUTE", "ACCESS", "ACTION", "ADA", "ADD", 
        "ADMIN", "AFTER", "AGGREGATE", "ALIAS", "ALL", "ALLOCATE", "ALSO", 
        "ALTER", "ALWAYS", "ANALYSE", "ANALYZE", "AND", "ANY", "ARE", "ARRAY|",
        "AS", "ASC", "ASENSITIVE", "ASSERTION", "ASSIGNMENT", "ASYMMETRIC", 
        "AT", "ATOMIC", "ATTRIBUTE", "ATTRIBUTES", "AUDIT", "AUTHORIZATION", 
        "AUTO_INCREMENT", "AVG", "AVG_ROW_LENGTH", 
        
        "BACKUP", "BACKWARD", "BEFORE", "BEGIN", "BERNOULLI", "BETWEEN", 
        "BIGINT|", "BINARY|", "BIT|", "BIT_LENGTH", "BITVAR", "BLOB|", "BOOL|", 
        "BOOLEAN|", "BOTH", "BREADTH", "BREAK", "BROWSE", "BULK", "BY",
        
        "C", "CACHE", "CALL", "CALLED", "CARDINALITY", "CASCADE", "CASCADED", 
        "CASE", "CAST", "CATALOG", "CATALOG_NAME", "CEIL", "CEILING", "CHAIN", 
        "CHANGE", "CHAR|","CHAR_LENGTH", "CHARACTER|", "CHARACTER_LENGTH", 
        "CHARACTER_SET_CATALOG", "CHARACTER_SET_NAME", "CHARACTER_SET_SCHEMA", 
        "CHARACTERISTIC", "CHARACTERS", "CHECK", "CHECKED", "CHECKPOINT", 
        "CHECKSUM", "CLASS", "CLASS_ORIGIN", "CLOB|", "CLOSE", "CLUSTER", 
        "CLUSTERED", "COALESCE", "COBOL", "COLLATE", "COLLATION", 
        "COLLATION_CATALOG", "COLLATION_NAME", "COLLATION_SCHEMA", "COLLECT", 
        "COLUMN", "COLUMN_NAME", "COLUMNS", "COMMAND_FUNCTION", 
        "COMMAND_FUNCTION_CODE", "COMMENT", "COMMIT", "COMMITTED", "COMPLETED",
        "COMPRESS", "COMPUTE", "CONDITION", "CONDITION_NUMBER", "CONNECT", 
        "CONNECTION", "CONNECTION_NAME", "CONSTRAINT", "CONSTRAINT_CATALOG",
        "CONSTRAINT_NAME", "CONSTRAINT_SCHEMA", "CONSTRAINTS", "CONSTRUCTOR",
        "CONTAINS", "CONTAINSTABLE", "CONTINUE", "CONVERSION", "COPY", "CORR",
        "CORRESPONDING", "COUNT", "COVAR_POP", "COVAR_SAMP", "CREATE", 
        "CREATEDB", "CREATEROLE", "CREATEUSER", "CROSS", "CSV","CUBE",
        "CUME_DIST", "CURRENT", "CURRENT_DATE", 
        "CURRENT_DEFAULT_TRANSFORM_GROUP", "CURRENT_PATH", "CURRENT_ROLE",
        "CURRENT_TIME", "CURRENT_TIMESTAMP", 
        "CURRENT_TRANSFORM_GROUP_FOR_TYPE", "CURRENT_USER", "CURSOR|",
        "CURSOR_NAME", "CYCLE", 
        
        "DATA", "DATABASE", "DATABASES", "DATE|", "DATETIME|", 
        "DATETIME_INTERVAL_CODE", "DATETIME_INTERVAL_PRECISION", "DAY", 
        "DAY_HOUR", "DAY_MICROSECOND", "DAY_MINUTE", "DAY_SECOND", 
        "DAYOFMONTH", "DAYOFWEEK", "DAYOFYEAR", "DBCC", "DEALLOCATE", "DEC",
        "DECIMAL|", "DECLARE", "DEFAULT", "DEFAULTS", "DEFERRABLE",
        "DEFINED", "DEFINER", "DEGREE", "DELAY_KEY_WRITE", "DELAYED", "DELETE",
        "DELIMITER", "DELIMITERS", "DENSE_RANK", "DENY", "DEPTH", "DEREF",
        "DERIVED", "DESC", "DESCRIBE", "DESCRIPTOR", "DESTROY", "DESTRUCTOR",
        "DETERMINISTIC", "DIAGNOSTIC", "DICTIONARY", "DISABLE", "DISCONNECT",
        "DISK", "DISPATCH", "DISTINCT", "DISTINCTROW", "DISTRIBUTED", "DIV",
        "DO", "DOMAIN", "DOUBLE|", "DROP", "DUAL", "DUMMY", "DUMP", "DYNAMIC",
        "DYNAMIC_FUNCTION", "DYNAMIC_FUNCTION_CODE", 
        
        "EACH", "ELEMENT", "ELSE", "ELSEIF", "ENABLE", "ENCLOSED", "ENCODING", 
        "ENCRYPTED", "END", "END-EXEC", "ENUM", "EQUALS", "ERRLVL", "ESCAPE",
        "EVERY", "EXCEPT", "EXCEPTION", "EXCLUDE", "EXCLUDING", "EXCLUSIVE",
        "EXEC", "EXECUTE", "EXISTING", "EXISTS", "EXIT", "EXP", "EXPLAIN",
        "EXTERNAL", "EXTRACT", 
        
        "FALSE|", "FETCH", "FIELDS", "FILE", "FILLFACTOR", "FILTER", "FINAL",
        "FIRST", "FLOAT|", "FLOAT4|", "FLOAT8|", "FLOOR", "FLUSH", "FOLLOWING", 
        "FOR", "FORCE", "FOREIGN", "FORTRAN", "FORWARD", "FOUND", "FREE",
        "FREETEXT", "FREETEXTTABLE", "FREEZE", "FROM", "FULL", "FULLTEXT", 
        "FUNCTION", "FUSION", 
        
        "G", "GENERAL", "GENERATED", "GET", "GLOBAL", "GO", "GOTO", "GRANT",
        "GRANTED", "GRANTS", "GREATEST", "GROUP", "GROUPING", 
        
        "HANDLER", "HAVING", "HEADER", "HEAP", "HIERARCHY", "HIGH_PRIORITY",
        "HOLD", "HOLDLOCK", "HOST", "HOSTS", "HOUR", "HOUR_MICROSECOND", 
        "HOUR_MINUTE", "HOUR_SECOND", 
        
        "IDENTIFIED", "IDENTITY", "IDENTITY_INSERT", "IDENTITYCOL", "IF",
        "IGNORE", "ILIKE", "IMMEDIATE", "IMMUTABLE", "IMPLEMENTATION",
        "IMPLICIT", "IN", "INCLUDE", "INCLUDING", "INCREMENT", "INDEX",
        "INDICATOR", "INFILE", "INFIX", "INHERIT", "INHERITS", "INITIAL",
        "INITIALIZE", "INITIALLY", "INNER", "INOUT", "INPUT", "INSENSITIVE",
        "INSERT", "INSERT_ID", "INSTANCE", "INSTANTIABLE", "INSTEAD", "INT|",
        "INT1|", "INT2|", "INT3|", "INT4|", "INT8|", "INTEGER|", "INTERSECT",
        "INTERSECTION", "INTERVAL", "INTO", "INVOKER", "IS", "ISAM", "ISNULL",
        "ISOLATION", "ITERATE", 
        
        "JOIN", 
        
        "K", "KEY", "KEY_MEMBER", "KEY_TYPE", "KEYS", "KILL", 
        
        "LANCOMPILER", "LANGUAGE", "LARGE", "LAST", "LAST_INSERT_ID",
        "LATERAL", "LEADING", "LEAST", "LEAVE", "LEFT", "LENGTH", "LESS",
        "LEVEL", "LIKE", "LIMIT", "LINENO", "LINES", "LISTEN", "LN", "LOAD",
        "LOCAL", "LOCALTIME", "LOCALTIMESTAMP", "LOCATION", "LOCATOR", "LOCK",
        "LOGIN", "LOGS", "LONG|", "LONGBLOB|", "LONGTEXT|", "LOOP",
        "LOW_PRIORITY", "LOWER", 
        
        "M", "MAP", "MATCH", "MATCHED", "MAX", "MAX_ROWS", "MAXETXTENTS",
        "MAXVALUE", "MEDIUMBLOB|", "MEDIUMINT|", "MEDIUMINT|", "MEMBER",
        "MERGE", "MESSAGE_LENGTH", "MESSAGE_OCTET_LENGTH", "MESSAGE_TEXT",
        "METHOD", "MIDDLEINT|", "MIN", "MIN_ROWS", "MINUS", "MINUTE",
        "MINUTE_MICROSECOND", "MINUTE_SECOND", "MINVALUE", "MLSLABEL",
        "MOD", "MODE", "MODIFIES", "MODIFY", "MODULE", "MONTH", "MONTHNAME",
        "MORE", "MOVE", "MULTISET", "MUMPS", "MYISAM", 
        
        "NAME", "NAMES", "NATIONAL", "NATURAL", "NCHAR|", "NCLOB|", "NESTING",
        "NEW", "NEXT", "NO", "NO_WRITE_TO_BINLOG", "NOAUDIT", "NOCHECK",
        "NOCOMPRESS", "NOCREATEDB", "NOCREATEROLE", "NOCREATEUSER",
        "NOINHERIT", "NOLOGIN", "NONCLUSTERED", "NONE", "NORMALIZE",
        "NORMALIZED", "NOSUPERUSER", "NOT", "NOTHING", "NOTIFY", "NOTNULL",
        "NOWAIT", "NULL", "NULLABLE", "NULLIF", "NULLS", "NUMBER|", "NUMERIC|", 
        
        "OBJECT", "OCTET_LENGTH", "OCTETS", "OF", "OFF", "OFFLINE", "OFFSET",
        "OFFSETS", "OIDS", "OLD", "ON", "ONLINE", "ONLY", "OPEN",
        "OPENDATASOURCE", "OPENQUERY", "OPENROWSET", "OPENXML", "OPERATION",
        "OPERATOR", "OPTIMIZE", "OPTION", "OPTIONALLY", "OPTIONS", "OR", 
        "ORDER", "ORDERING", "ORDINALITY", "OTHERS", "OUT", "OUTER", "OUTFILE",
        "OUTPUT", "OVER", "OVERLAPS", "OVERLAY", "OVERRIDING", "OWNER",
        
        "PACK_KEYS", "PAD", "PARAMETER", "PARAMETER_MODE", "PARAMETER_NAME", 
        "PARAMETER_ORDINAL_POSITION", "PARAMETER_SPECIFIC_CATALOG", 
        "PARAMETER_SPECIFIC_NAME", "PARAMETER_SPECIFIC_SCHEMA", "PARAMETERS",
        "PARTIAL", "PARTITION", "PASCAL", "PASSWORD", "PATH", "PCTFREE",
        "PERCENT", "PERCENT_RANK", "PERCENTILE_CONT", "PERCENTILE_DISC",
        "PLACING" "PLAN", "PLI", "POSITION", "POSTFIX", "POWER", "PRECEDING",
        "PRECISION", "PREFIX", "PREORDER", "PREPARE", "PREPARED", "PRESERVE",
        "PRIMARY", "PRINT", "PRIOR", "PRIVILEGES", "PROC", "PROCEDURAL",
        "PROCEDURE", "PROCESS", "PROCESSLIST", "PUBLIC", "PURGE",
        
        "RAID0", "RAISERROR", "RANGE", "RANK", "RAW", "READ", "READS",
        "READTEXT", "|REAL", "RECHECK", "RECONFIGURE", "RECURSIVE", "REF",
        "REFERENCES", "REFERENCING", "REGEXP", "REGR_AVGX", "REGR_AVGY",
        "REGR_COUNT", "REGR_INTERCEPT", "REGR_R2", "REGR_SLOPE", "REGR_SXX",
        "REGR_SXY", "REGR_SYY", "REINDEX", "RELATIVE", "RELEASE", "RELOAD",
        "RENAME", "REPEAT", "REPEATABLE", "REPLACE", "REPLICATION", "REQUIRE",
        "RESET", "RESIGNAL", "RESOURCE", "RESTART", "RESTORE", "RESTRICT", 
        "RESULT", "RETURN", "RETURNED_CARDINALITY", "RETURNED_LENGTH", 
        "RETURNED_OCTET_LENGTH", "RETURNED_SQLSTATE", "RETURNS", "REVOKE",
        "RIGHT", "RLIKE", "ROLE", "ROLLBACK", "ROLLUP", "ROUTINE",
        "ROUTINE_CATALOG", "ROUTINE_NAME", "ROUTINE_SCHEMA", "ROW", 
        "ROW_COUNT", "ROW_NUMBER", "ROWCOUNT", "ROWGUIDCOL", "ROWID", "ROWNUM",
        "ROWS", "RULE", 
        
        "SAVE", "SAVEPOINT", "SCALE", "SCHEMA", "SCHEMA_NAME", "SCHEMAS", 
        "SCOPE", "SCOPE_CATALOG", "SCOPE_NAME", "SCROLL", "SEARCH", "SECOND",
        "SECOND_MICROSECOND", "SECTION", "SECURITY", "SELECT", "SELF", 
        "SENSITIVE", "SEPARATOR", "SEQUENCE", "SERIALIZABLE", "SERVER_NAME",
        "SESSION", "SESSION_USER", "SET", "SETOF", "SETS", "SETUSER", "SHARE",
        "SHOW", "SHUTDOWN", "SIGNAL", "SIMILAR", "SIMPLE", "SIZE", "SMALLINT|",
        "SOME", "SONAME", "SOURCE", "SPACE", "SPATIAL", "SPECIFIC", 
        "SPECIFIC_NAME", "SPECIFICTYPE", "SQL", "SQL_BIG_RESULT",
        "SQL_BIG_SELECTS", "SQL_BIG_TABLES", "SQL_CALC_FOUND_ROWS", 
        "SQL_LOG_OFF", "SQL_LOG_UPDATE", "SQL_LOW_PRIORITY_UPDATES",
        "SQL_SELECT_LIMIT", "SQL_SMALL_RESULT", "SQL_WARNINGS", "SQLCA",
        "SQLCODE", "SQLERROR", "SQLEXCEPTION", "SQLSTATE", "SQLWARNING", 
        "SQRT", "SSL", "STABLE", "START", "STARTING", "STATE", "STATEMENT",
        "STATIC", "STATISTIC", "STATUS", "STDDEV_POP", "STDDEV_SAMP", "STDIN",
        "STDOUT", "STORAGE", "STRAIGHT_JOIN", "STRICT", "STRING", "STRUCTURE",
        "STYLE", "SUBCLASS_ORIGIN", "SUBLIST", "SUBMULTISET", "SUBSTRING",
        "SUCCESSFUL", "SUM", "SUPERUSER", "SYMMETRIC", "SYNONYM", "SYSDATE",
        "SYSID", "SYSTEM", "SYSTEM_USER", 
        
        "TABLE", "TABLE_NAME", "TABLES", "TABLESAMPLE", "TABLESPACE", "TEMP",
        "TEMPLATE", "TEMPORARY", "TERMINATE", "TERMINATED", "TEXT|", 
        "TEXTSIZE", "THAN", "THEN", "TIES", "TIME|", "TIMESTAMP|", 
        "TIMEZONE_HOUR", "TIMEZONE_MINUTE", "TINYBLOB|", "TINYINT|", 
        "TINYTEXT|", "TO", "TOAST", "TOP", "TOP_LEVEL_COUNT", "TRAILING", 
        "TRAN", "TRANSACTION", "TRANSACTION_ACTIVE", "TRANSACTIONS_COMMITTED",
        "TRANSACTIONS_ROLLED_BACK", "TRANSFORM", "TRANSFORMS", "TRANSLATE",
        "TREAT", "TRIGGER", "TRIGGER_CATALOG", "TRIGGER_NAME", 
        "TRIGGER_SCHEMA", "TRIM", "TRUE|", "TRUNCATE", "TRUSTED", "TSEQUAL",
        "TYPE", "UESCAPE", "|UID", "UNBOUNDED", "UNCOMMITTED", "UNDER", "UNDO",
        "UNENCRYPTED", "UNION", "UNIQUE", "UNKNOWN", "UNLISTEN", "UNLOCK",
        "UNNAMED", "UNNEST", "UNSIGNED|", "UNTIL", "UPDATE", "UPDATETEXT",
        "UPPER", "USAGE", "USE", "USER", "USER_DEFINED_TYPE_CATALOG",
        "USER_DEFINED_TYPE_CODE", "USER_DEFINED_TYPE_NAME", 
        "USER_DEFINED_TYPE_SCHEMA", "USING", "UTC_DATE", "UTC_TIME", 
        "UTC_TIMESTAMP", 
        
        "VACUUM", "VALID", "VALIDATE", "VALIDATOR", "VALUE", "VALUES", 
        "VAR_POP", "VAR_SAMP", "VARBINARY|", "VARCHAR|", "VARCHAR2|", 
        "VARCHARACTER|", "VARIABLE", "VARIABLES", "VARYING", "VERBOSE", 
        "VIEW", "VOLATILE", 
        
        "WAITFOR", "WHEN", "WHENEVER", "WHERE", "WHILE", "WIDTH_BUCKET",
        "WINDOW", "WITH", "WITHIN", "WITHOUT", "WORK", "WRITE", "WRITETEXT",
        "X509", "XOR",
        
        "YEAR", "YEAR_MONTH", 
        
	/* lowercase */

        "zerofill", "zone",
        "a", "abort", "abs", "absolute", "access", "action", "ada", "add", 
        "admin", "after", "aggregate", "alias", "all", "allocate", "also", 
        "alter", "always", "analyse", "analyze", "and", "any", "are", "array|",
        "as", "asc", "asensitive", "assertion", "assignment", "asymmetric", 
        "at", "atomic", "attribute", "attributes", "audit", "authorization", 
        "auto_increment", "avg", "avg_row_length", 
        
        "backup", "backward", "before", "begin", "bernoulli", "between", 
        "bigint|", "binary|", "bit|", "bit_length", "bitvar", "blob|", "bool|", 
        "boolean|", "both", "breadth", "break", "browse", "bulk", "by",
        
        "c", "cache", "call", "called", "cardinality", "cascade", "cascaded", 
        "case", "cast", "catalog", "catalog_name", "ceil", "ceiling", "chain", 
        "change", "char|","char_length", "character|", "character_length", 
        "character_set_catalog", "character_set_name", "character_set_schema", 
        "characteristic", "characters", "check", "checked", "checkpoint", 
        "checksum", "class", "class_origin", "clob|", "close", "cluster", 
        "clustered", "coalesce", "cobol", "collate", "collation", 
        "collation_catalog", "collation_name", "collation_schema", "collect", 
        "column", "column_name", "columns", "command_function", 
        "command_function_code", "comment", "commit", "committed", "completed",
        "compress", "compute", "condition", "condition_number", "connect", 
        "connection", "connection_name", "constraint", "constraint_catalog",
        "constraint_name", "constraint_schema", "constraints", "constructor",
        "contains", "containstable", "continue", "conversion", "copy", "corr",
        "corresponding", "count", "covar_pop", "covar_samp", "create", 
        "createdb", "createrole", "createuser", "cross", "csv","cube",
        "cume_dist", "current", "current_date", 
        "current_default_transform_group", "current_path", "current_role",
        "current_time", "current_timestamp", 
        "current_transform_group_for_type", "current_user", "cursor|",
        "cursor_name", "cycle", 
        
        "data", "database", "databases", "date|", "datetime|", 
        "datetime_interval_code", "datetime_interval_precision", "day", 
        "day_hour", "day_microsecond", "day_minute", "day_second", 
        "dayofmonth", "dayofweek", "dayofyear", "dbcc", "deallocate", "dec",
        "decimal|", "declare", "default", "defaults", "deferrable",
        "defined", "definer", "degree", "delay_key_write", "delayed", "delete",
        "delimiter", "delimiters", "dense_rank", "deny", "depth", "deref",
        "derived", "desc", "describe", "descriptor", "destroy", "destructor",
        "deterministic", "diagnostic", "dictionary", "disable", "disconnect",
        "disk", "dispatch", "distinct", "distinctrow", "distributed", "div",
        "do", "domain", "double|", "drop", "dual", "dummy", "dump", "dynamic",
        "dynamic_function", "dynamic_function_code", 
        
        "each", "element", "else", "elseif", "enable", "enclosed", "encoding", 
        "encrypted", "end", "end-exec", "enum", "equals", "errlvl", "escape",
        "every", "except", "exception", "exclude", "excluding", "exclusive",
        "exec", "execute", "existing", "exists", "exit", "exp", "explain",
        "external", "extract", 
        
        "false|", "fetch", "fields", "file", "fillfactor", "filter", "final",
        "first", "float|", "float4|", "float8|", "floor", "flush", "following", 
        "for", "force", "foreign", "fortran", "forward", "found", "free",
        "freetext", "freetexttable", "freeze", "from", "full", "fulltext", 
        "function", "fusion", 
        
        "g", "general", "generated", "get", "global", "go", "goto", "grant",
        "granted", "grants", "greatest", "group", "grouping", 
        
        "handler", "having", "header", "heap", "hierarchy", "high_priority",
        "hold", "holdlock", "host", "hosts", "hour", "hour_microsecond", 
        "hour_minute", "hour_second", 
        
        "identified", "identity", "identity_insert", "identitycol", "if",
        "ignore", "ilike", "immediate", "immutable", "implementation",
        "implicit", "in", "include", "including", "increment", "index",
        "indicator", "infile", "infix", "inherit", "inherits", "initial",
        "initialize", "initially", "inner", "inout", "input", "insensitive",
        "insert", "insert_id", "instance", "instantiable", "instead", "int|",
        "int1|", "int2|", "int3|", "int4|", "int8|", "integer|", "intersect",
        "intersection", "interval", "into", "invoker", "is", "isam", "isnull",
        "isolation", "iterate", 
        
        "join", 
        
        "k", "key", "key_member", "key_type", "keys", "kill", 
        
        "lancompiler", "language", "large", "last", "last_insert_id",
        "lateral", "leading", "least", "leave", "left", "length", "less",
        "level", "like", "limit", "lineno", "lines", "listen", "ln", "load",
        "local", "localtime", "localtimestamp", "location", "locator", "lock",
        "login", "logs", "long|", "longblob|", "longtext|", "loop",
        "low_priority", "lower", 
        
        "m", "map", "match", "matched", "max", "max_rows", "maxetxtents",
        "maxvalue", "mediumblob|", "mediumint|", "mediumint|", "member",
        "merge", "message_length", "message_octet_length", "message_text",
        "method", "middleint|", "min", "min_rows", "minus", "minute",
        "minute_microsecond", "minute_second", "minvalue", "mlslabel",
        "mod", "mode", "modifies", "modify", "module", "month", "monthname",
        "more", "move", "multiset", "mumps", "myisam", 
        
        "name", "names", "national", "natural", "nchar|", "nclob|", "nesting",
        "new", "next", "no", "no_write_to_binlog", "noaudit", "nocheck",
        "nocompress", "nocreatedb", "nocreaterole", "nocreateuser",
        "noinherit", "nologin", "nonclustered", "none", "normalize",
        "normalized", "nosuperuser", "not", "nothing", "notify", "notnull",
        "nowait", "null", "nullable", "nullif", "nulls", "number|", "numeric|", 
        
        "object", "octet_length", "octets", "of", "off", "offline", "offset",
        "offsets", "oids", "old", "on", "online", "only", "open",
        "opendatasource", "openquery", "openrowset", "openxml", "operation",
        "operator", "optimize", "option", "optionally", "options", "or", 
        "order", "ordering", "ordinality", "others", "out", "outer", "outfile",
        "output", "over", "overlaps", "overlay", "overriding", "owner",
        
        "pack_keys", "pad", "parameter", "parameter_mode", "parameter_name", 
        "parameter_ordinal_position", "parameter_specific_catalog", 
        "parameter_specific_name", "parameter_specific_schema", "parameters",
        "partial", "partition", "pascal", "password", "path", "pctfree",
        "percent", "percent_rank", "percentile_cont", "percentile_disc",
        "placing" "plan", "pli", "position", "postfix", "power", "preceding",
        "precision", "prefix", "preorder", "prepare", "prepared", "preserve",
        "primary", "print", "prior", "privileges", "proc", "procedural",
        "procedure", "process", "processlist", "public", "purge",
        
        "raid0", "raiserror", "range", "rank", "raw", "read", "reads",
        "readtext", "|real", "recheck", "reconfigure", "recursive", "ref",
        "references", "referencing", "regexp", "regr_avgx", "regr_avgy",
        "regr_count", "regr_intercept", "regr_r2", "regr_slope", "regr_sxx",
        "regr_sxy", "regr_syy", "reindex", "relative", "release", "reload",
        "rename", "repeat", "repeatable", "replace", "replication", "require",
        "reset", "resignal", "resource", "restart", "restore", "restrict", 
        "result", "return", "returned_cardinality", "returned_length", 
        "returned_octet_length", "returned_sqlstate", "returns", "revoke",
        "right", "rlike", "role", "rollback", "rollup", "routine",
        "routine_catalog", "routine_name", "routine_schema", "row", 
        "row_count", "row_number", "rowcount", "rowguidcol", "rowid", "rownum",
        "rows", "rule", 
        
        "save", "savepoint", "scale", "schema", "schema_name", "schemas", 
        "scope", "scope_catalog", "scope_name", "scroll", "search", "second",
        "second_microsecond", "section", "security", "select", "self", 
        "sensitive", "separator", "sequence", "serializable", "server_name",
        "session", "session_user", "set", "setof", "sets", "setuser", "share",
        "show", "shutdown", "signal", "similar", "simple", "size", "smallint|",
        "some", "soname", "source", "space", "spatial", "specific", 
        "specific_name", "specifictype", "sql", "sql_big_result",
        "sql_big_selects", "sql_big_tables", "sql_calc_found_rows", 
        "sql_log_off", "sql_log_update", "sql_low_priority_updates",
        "sql_select_limit", "sql_small_result", "sql_warnings", "sqlca",
        "sqlcode", "sqlerror", "sqlexception", "sqlstate", "sqlwarning", 
        "sqrt", "ssl", "stable", "start", "starting", "state", "statement",
        "static", "statistic", "status", "stddev_pop", "stddev_samp", "stdin",
        "stdout", "storage", "straight_join", "strict", "string", "structure",
        "style", "subclass_origin", "sublist", "submultiset", "substring",
        "successful", "sum", "superuser", "symmetric", "synonym", "sysdate",
        "sysid", "system", "system_user", 
        
        "table", "table_name", "tables", "tablesample", "tablespace", "temp",
        "template", "temporary", "terminate", "terminated", "text|", 
        "textsize", "than", "then", "ties", "time|", "timestamp|", 
        "timezone_hour", "timezone_minute", "tinyblob|", "tinyint|", 
        "tinytext|", "to", "toast", "top", "top_level_count", "trailing", 
        "tran", "transaction", "transaction_active", "transactions_committed",
        "transactions_rolled_back", "transform", "transforms", "translate",
        "treat", "trigger", "trigger_catalog", "trigger_name", 
        "trigger_schema", "trim", "true|", "truncate", "trusted", "tsequal",
        "type", "uescape", "|uid", "unbounded", "uncommitted", "under", "undo",
        "unencrypted", "union", "unique", "unknown", "unlisten", "unlock",
        "unnamed", "unnest", "unsigned|", "until", "update", "updatetext",
        "upper", "usage", "use", "user", "user_defined_type_catalog",
        "user_defined_type_code", "user_defined_type_name", 
        "user_defined_type_schema", "using", "utc_date", "utc_time", 
        "utc_timestamp", 
        
        "vacuum", "valid", "validate", "validator", "value", "values", 
        "var_pop", "var_samp", "varbinary|", "varchar|", "varchar2|", 
        "varcharacter|", "variable", "variables", "varying", "verbose", 
        "view", "volatile", 
        
        "waitfor", "when", "whenever", "where", "while", "width_bucket",
        "window", "with", "within", "without", "work", "write", "writetext",
        "x509", "xor",
        
        "year", "year_month", 
        
        "zerofill", "zone",
        
        NULL
};

char *nginx_HL_extensions[] = { "nginx.conf", "global.conf", NULL };
char *nginx_HL_keywords[] = {
        /* http://nginx.org/en/docs/dirindex.html */
        "absolute_redirect", "accept_mutex", "accept_mutex_delay", 
        "access_log", "add_after_body", "add_before_body", "add_header",
        "add_trailer", "addition_types", "aio", "aio_write", "alias", "allow",
        "ancient_browser", "ancient_browser_value", "api", "auth_basic",
        "auth_basic_user_file", "auth_http", "auth_http_header", 
        "auth_http_pass_client_cert", "auth_http_timeout", "auth_jwt", 
        "auth_jwt_claim_set", "auth_jwt_key_file", "auth_request", 
        "auth_request_set", "autoindex", "autoindex_exact_size", 
        "autoindex_format", "autoindex_localtime", 
        
        "break",
        
        "charset", "charset_map", "charset_types", "chunked_transfer_encoding",
        "client_body_buffer_size", "client_body_in_file_only", 
        "client_body_in_single_buffer", "client_body_temp_path", 
        "client_body_timeout", "client_header_buffer_size", 
        "client_header_timeout", "client_max_body_size", 
        "connection_pool_size", "create_full_put_path",
        
        "daemon", "dav_access", "dav_methods", "debug_connection", 
        "debug_point", "default_type", "deny", "directio", 
        "directio_alignment", "diable_symlinks",
        
        "empty_gif", "env", "error_log", "error_page", "etag", "events", 
        "expires", 
        
        "f4f", "f4f_buffer_size", "fastcgi_buffer_size", "fastcgi_buffering",
        "fastcgi_buffers", "fastcgi_busy_buffers_size", "fastcgi_cache", 
        "fastcgi_cache_background_update", "fastcgi_cache_bypass",
        "fastcgi_cache_key", "fastcgi_cache_lock", "fastcgi_cache_lock_age",
        "fastcgi_cache_lock_timeout", "fastcgi_cache_max_range_offset",
        "fastcgi_cache_methods", "fastcgi_cache_min_users", 
        "fastcgi_cache_path", "fastcgi_cache_purge", 
        "fastcgi_cache_revalidate", "fastcgi_cache_use_stale",
        "fastcgi_cache_valid", "fastcgi_catch_stderr",
        "fastcgi_connect_timeout", "fastcgi_force_ranges", 
        "fastcgi_hide_header", "fastcgi_ignore_client_abort", 
        "fastcgi_ignore_headers", "fastcgi_index", "fastcgi_intercept_errors",
        "fastcgi_keep_conn", "fastcgi_limit_rate", 
        "fastcgi_max_temp_file_size", "fastcgi_next_upstream",
        "fastcgi_next_upstream_timeout", "fastcgi_next_upstream_tries",
        "fastcgi_no_cache", "fastcgi_param", "fastcgi_pass", 
        "fastcgi_pass_header", "fastcgi_pass_request_body", 
        "fastcgi_pass_request_headers", "fastcgi_read_timeout", 
        "fastcgi_request_buffering", "fastcgi_send_lowat", 
        "fastcgi_send_timeout", "fastcgi_split_path_info", "fastcgi_store",
        "fastcgi_store_access", "fastcgi_temp_file_write_size", 
        "fastcgi_temp_path", "flv",
        
        "geo", "geoip_city", "geoip_country", "geoip_org", "geoip_proxy",
        "geoip_proxy_recursive", "google_perftools_profiles", "gunzip",
        "gunzip_buffers", "gzip", "gzip_buffers", "gzip_comp_level",
        "gzip_disable", "gzip_http_version", "gzip_min_length", "gzip_proxied",
        "gzip_static", "gzip_types", "gzip_vary", 
        
        "hash", "health_check", "health_check_timeout", "hls", "hls_buffers",
        "hls_forward_args", "hls_fragment", "hls_mp4_buffer_size",
        "hls_mp4_max_buffer_size", "http", "http2_body_preread_size",
        "http2_idle_timeout", "http2_max_concurrent_streams", 
        "http2_max_field_size", "http2_max_header_size", "http2_max_requests",
        "http2_recv_buffer_size", "http2_recv_timeout",
        
        "if", "if_modified_since", "ignore_invalid_headers", "image_filter",
        "image_filter_buffer", "image_filter_interface",
        "image_filter_interlace", "image_filter_jpeg_quality",
        "image_filter_sharpen", "image_filter_transparency", 
        "image_filter_webp_quality", "imap_auth", "imap_capabilities", 
        "imap_client_buffer", "include", "index", "internal", "ip_hash",
        
        "js_access", "js_content", "js_filter", "js_include", "js_preread",
        "js_set", 
        
        "keepalive", "keepalive_disable", "keepalive_requests", 
        "keepalive_timeout", "keyval", "keyval_zone",
        
        "large_client_header_buffers", "least_conn", "least_time", 
        "Limit_conn", "limit_conn_log_level", "limit_conn_status", 
        "limit_conn_status", "limit_conn_zone", "limit_except", "Limit_rate",
        "limit_rate_after", "limit_req", "limit_req_log_level",
        "limit_req_status", "limit_req_zone", "limit_zone", "lingering_close",
        "lingering_time", "lingering_timeout", "listen", "load_module", 
        "location", "lock_file", "log_format", "log_not_found", 
        "log_subrequest",
        
        "mail", "map", "map_hash_bucket_size", "map_hash_max_size",
        "master_process", "match", "max_ranges", "memcached_bind", 
        "memcached_buffer_size", "memcached_connect_timeout",
        "memcached_force_ranges", "memcached_gzip_flag", 
        "memcached_next_upstream", "memcached_net_upstream_timeout",
        "memcached_next_upstream_tries", "memcached_pass", 
        "memcached_read_timeout", "memcahced_send_timeout", "merge_slashes",
        "min_delete_depth", "mirror", "mirror_request_body", "modern_browser",
        "modern_browser_value", "mp4", "mp4_buffer_size", "mp4_limit_rate",
        "mp4_limit_rate_after", "mp4_max_buffer_size", "msie_padding",
        "msie_refresh", "multi_accept", 
        
        "ntim",
        
        "open_file_cache", "open_file_cache_errors", 
        "open_file_cache_min_uses", "open_file_cache_valid", 
        "open_log_file_cache", "output_buffers", "override_charset",
        
        "pcre_jit", "perl", "perl_modules", "perl_require", "perl_set", "pid",
        "pop3_auth", "pop3_capabilities", "port_in_redirect", 
        "postpone_output", "preread_buffer_size", "preread_timeout", 
        "protocol", "proxy_bind", "proxy_buffer", "proxy_buffer_size", 
        "proxy_buffering", "proxy_buffers", "proxy_busy_buffers_size", 
        "proxy_cache", "proxy_cache_background_update", "proxy_cache_bypass",
        "proxy_cache_convert_head", "proxy_cache_key", "proxy_cache_key",
        "proxy_cache_lock", "proxy_cache_lock_age", "proxy_cache_lock_timeout",
        "proxy_cache_max_range_offset", "proxy_cache_methods",
        "proxy_cache_min_uses", "proxy_cache_path", "proxy_cache_purge",
        "proxy_cache_revalidate", "proxy_cache_use_stale", "proxy_cache_valid",
        "proxy_connect_timeout", "proxy_cookie_domain", "proxy_cookie_path",
        "proxy_download_rate", "proxy_force_ranges", 
        "proxy_headers_hash_bucket_size", "proxy_headers_hash_max_size",
        "proxy_hide_header", "proxy_http_version", "proxy_ignore_client_abort",
        "proxy_ignore_headers", "proxy_intercept_errors", "proxy_limit_rate",
        "proxy_max_temp_file_size", "proxy_method", "proxy_next_upstream",
        "proxy_next_upstream_timeout", "proxy_next_upstrean_tries", 
        "prixy_no_cache", "proxy_pass", "proxy_pass_error_message", 
        "proxy_pass_header", "proxy_pass_rquest_body", 
        "proxy_pass_request_headers", "proxy_protocol", 
        "proxy_protocol_timeout", "proxy_read_timeout", "proxy_redirect", 
        "proxy_request_buffering", "proxy_responses", "proxy_send_lowat", 
        "proxy_send_timeout", "proxy_set_body", "proxy_set_header",
        "proxy_ssl", "proxy_ssl_certificate", "proxy_ssl_certificate_key",
        "proxy_ssl_ciphers", "proxy_ssl_crl", "proxy_ssl_name", 
        "proxy_ssl_password_file", "proxy_ssl_protocols",
        "proxy_ssl_server_name", "proxy_ssl_session_reuse",
        "proxy_ssl_trusted_certificate", "proxy_ssl_verify", 
        "proxy_ssl_verify_depth", "proxy_store", "proxy_store_access", 
        "proxy_temp_file_write_size", "proxy-temp_path", "proxy_timeout", 
        "proxy_upload_rate",
        
        "queue", 
        
        "random_index", "read_ahead", "real_ip_header","real_ip_recursive",
        "recursive_error_pages", "referer_hash_bucket_size",
        "referer_hash_max_size", "request_pool_size",
        "reset_timeout_connection", "resolver", "resolver_timeout", "return",
        "rewrite", "rewrite_log", "root",
        
        "satisfy", "scgi_bind", "scgi_buffer_size", "scgi_buffering", 
        "scgi_buffers", "scgi_busy_buffers_size", "scgi_cache", 
        "scgi_cache_background_update", "scgi_cache_bypass", "scgi_cache_key",
        "scgi_cache_lock", "scgi_cache_lock_age", "scgi_cache_lock_timeout", 
        "scgi_cache_max_range_offset", "scgi_cache_methods", "scgi_cache_path",
        "scgi_cache_purge", "scgi_cache_revalidate", "scgi_cache_use_state",
        "scgi_cache_valid", "scgi_connect_timeout", "scgi_force_ranges", 
        "scgi_hide_header", "scgi_ignore_client_abort", "scgi_ignore_headers",
        "scgi_intercept_errors", "scgi_limit_rate", "scgi_max_temp_file_size",
        "scgi_next_upstream", "scgi_next_upstream_timeout",
        "scgi_next_upstream_tries", "scgi_no_cache", "scgi_param", "scgi_pass",
        "scgi_pass_header", "scgi_pass_request_body",
        "scgi_pass_request_headers", "scgi_read_timeout", 
        "scgi_request_buffering", "scgi_send_timeout", "scgi_store", 
        "scgi_store_access", "scgi_temp_file_write_size", "scgi_temp_path",
        "secure_link", "secure_link_md5", "secure_link_secret", "send_lowat",
        "send_timeout", "sendfile", "sendfile_max_chunk", "server",
        "server_name", "server_name_in_redirect", 
        "server_names_hash_bucket_size", "server_names_hash_max_size", 
        "server_tokens", "session_log", "session_log_format",
        "session_log_zone", "set", "set_real_ip_from", "slice", "smtp_auth",
        "smtp_capabilities", "source_charset", "spdy_chunk_size",
        "sdpy_headers_comp", "split_clients", "ssi", "ssi_last_modified",
        "ssi_min_file_chunk", "ssi_silent_errors", "ssi_types",
        "ssi_value_length", "ssl", "ssl_buffer_size", "ssl_certificate", 
        "ssl_certificate_key", "ssl_ciphers", "ssl_client_certificate",
        "ssl_crl", "ssl_dhparam", "ssl_ecdh_curve", "ssl_engine", 
        "ssl_handshake_timeout", "ssl_password_file",
        "ssl_prefer_server_ciphers", "ssl_preread", "ssl_protocols", 
        "ssl_session_cache", "ssl_session_ticket_key", "ssl_session_tickets",
        "ssl_session_timeout", "ssl_stapling", "ssl_stapling_file", 
        "ssl_stapling_responder", "ssl_stapling_verify",
        "ssl_trusted_certificate", "ssl_verify_client", "starttls", "state",
        "status", "status_format", "status_zone", "sticky", 
        "sticky_cookie_name", "stream", "stub_status", "sub_filter", 
        "sub_filter_last_modified", "sub_filter_once", "sub_filter_types", 
        
        "tcp_nodelay", "tcp_nopush", "thread_pool", "timeout",
        "timer_recolution", "try_files", "types", "types_hash_bucket_size",
        "types_hash_max_size", 
        
        "underscores_in_headers", "uninitialized_variable_warn", "upstream",
        "upstream_conf", "use", "user", "userid", "userid_domain", 
        "userid_expires", "userid_mark", "userid_name", "userid_p3p", 
        "userid_path", "userid_service", "uwsgi_bind", "uwsgi_buffer_size",
        "uwsgi_buffering", "uwsgi_buffers", "uwsgi_busy_buffers_size", 
        "uwsgi_cache", "uwsgi_cache_background_update", "uwsgi_cache_bypass",
        "uwsgi_cache_key", "uwsgi_cache_lock", "uwsgi_cache_lock_age",
        "uwsgi_cache_lock_timeout", "uwsgi_cache_max_range_offset", 
        "uwsgi_cache_methods", "uwsgi_cache_min_uses", "uwsgi_cache_path",
        "uwsgi_cache_purge", "uwsgi_cache_revalidate", "uwsgi_cache_use_stale",
        "uwsgi_cache_valid", "uwsgi_connect_timeout", "uwsgi_force_ranges",
        "uwsgi_hide_header", "uwsgi_ignore_client_abort",
        "uwsgi_ignore_headers", "uwsgi_intercept_errors", "uwsgi_limit_rate",
        "uwsgi_max_temp_file_size", "uwsgi_modifier1", "uwsgi_modifier2", 
        "uwsgi_next_upstream", "uwsgi_next_upstream_timeout", 
        "uwsgi_next_upstream_tries", "uwsgi_no_cache", "uwsgi_param",
        "uwsgi_pass", "uwsgi_pass_header", "uwsgi_pass_request_body",
        "uwsgi_pass_request_headers", "uwsgi_read_timeout",
        "uwsgi_request_buffering", "uwsgi_send_timeout",
        "uwsgi_ssl_certificate", "uwsgi_ssl_certificate_key", 
        "uwsgi_ssl_ciphers", "uwsgi_ssl_crl", "uwsgi_ssl_name", 
        "uwsgi_ssl_password_file", "uwsgi_ssl_protocols",
        "uwsgi_ssl_server_name", "uwsgi_ssl_session_reuse",
        "uwsgi_ssl_trusted_certificate", "uwsgi_ssl_verify", 
        "uwsgi_ssl_verify_depth", "uwsgi_store", "uwsgi_store_access", 
        "uwsgi_temp_file_write_size", "uwsgi_temp_path",
        
        "valid_referers", "variables_hash_bucket_size",
        "variables_hash_max_size", 
        
        "worker_aio_requests", "worker_connections", "worker_cpu_affinity",
        "worker_priority", "worker_processes", "worker_rlimit_core",
        "worker_rlimit_nofile", "worker_shutdown_timeout",
        "working_directory", 
        
        "xclient", "xml_entities", "xslt_last_modified", "xslt_param",
        "xslt_string_param", "xslt_stylesheet", "xslt_types", 
        
        "zone",
        
        NULL
};

char *go_HL_extensions[] = { ".go", NULL };
char *go_HL_keywords[] = {
        "break",
        "case", "chan", "const", "continue",
        "default", "defer", 
        "else", 
        "fallthrough", "for", "func",
        "go", "goto", 
        "if", "import", "interface",
        "map",
        "package",
        "range", "return",
        "select", "struct", "switch",
        "type",
        "var",
        
        "bool|", "byte|", // byte is alias for uint8
        "complex64|", "complex128|",
        "float32|", "float64|", 
        "int|", "int8|", "int16|", "int32|", "int64|", 
        "rune|", // alias for int32, represents a Unicode code point
        "string|",
        "uint|", "uint8|", "uint16|", "uint32|", "uint64|", "uinptr|",
        
        NULL
        
};

struct editor_syntax HLDB[] = {
	{
		"Text",
		Text_HL_extensions,
		Text_HL_keywords,
		"#", 
		"", "", 
		HARD_TABS,
		DEFAULT_KILO_TAB_STOP, /* tab stop */
		0  /* auto */
	},
	{
		"Makefile",
		Makefile_HL_extensions,
		Makefile_HL_keywords, 
		"#",
		"", "", /* Comment continuation by backslash is missing. */
		HARD_TABS,
		DEFAULT_KILO_TAB_STOP,
		1 /* auto indent */
	},
	{
		"C", 
		C_HL_extensions, 
		C_HL_keywords,
		"//", 
		"/*", "*/",
		HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
		DEFAULT_KILO_TAB_STOP,
		1, /* auto indent */
	},
	{
		"Java", 
		Java_HL_extensions, 
		Java_HL_keywords,
		"//", 
		"/*", "*/",
		HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
		4,
		1
	},
	{
		"Python",
		Python_HL_extensions,
		Python_HL_keywords,
		"#",
		"'''", "'''",
		HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
		4,
		1
	},
        {
                "Erlang",
                Erlang_HL_extensions, 
                Erlang_HL_keywords,
                "%",
                "", "",
                HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
                4,
                1 // TODO make bitfield?
        },
        {
                "JavaScript",
                JS_HL_extensions,
                JS_HL_keywords,
                "//",
                "/*", "*/",
                HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
                4,
                1
        },
        {
        	"Shell",
        	Shell_HL_extensions,
        	Shell_HL_keywords,
        	"#",
        	"", "",
        	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
        	8,
        	1
        },
        {
        	"Perl", 
        	Perl_HL_extensions,
        	Perl_HL_keywords,
        	"#", 
        	"", "", /* ^= ^= comments missing */
        	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
        	4,
        	1
        },
        {
                "Ruby",
                Ruby_HL_extensions, 
                Ruby_HL_keywords,
                "#",
                "", "",
                HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
                4,
                1
        },
        {
                "PHP",
                PHP_HL_extensions, 
                PHP_HL_keywords,
                "//", // TODO also '#'
                "/*", "*/",
                HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
                4,
                1
        },
        {
                "Elm",
                Elm_HL_extensions,
                Elm_HL_keywords,
                "--",
                "", "",
                HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
                4,
                1                
        },
        {
                "Bazel",
                Bazel_HL_extensions,
                Bazel_HL_keywords,
                "#",
                "", "",
                HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
                4,
                1
        },
        {
                "Docker",
                Dockerfile_HL_extensions,
                Dockerfile_HL_keywords,
                "#",
                "", "",
                HL_HIGHLIGHT_STRINGS, /* FIXME number parsing: ubuntu:16.04 only hilights 04 as number.*/
                4,
                1
        },
        {
                "SQL",
                SQL_HL_extensions,
                SQL_HL_keywords,
                "--",
                "/*", "*/",
                HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
                4,
                1       
        },
        {
                "nginx",
                nginx_HL_extensions,
                nginx_HL_keywords,
                "#",
                "", "", 
                HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
                4, 
                1      
        },
        {
                "go",
                go_HL_extensions,
                go_HL_keywords,
                "//",
                "/*", "*/", 
                HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS,
                4, 
                1      
        }
};

/* macro replacement */
int
hldb_entries() {
        return sizeof(HLDB) / sizeof(HLDB[0]);        
}

//#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))
 
