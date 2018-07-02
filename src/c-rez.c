/*
 * c-rez is a portable C90 tool that reads from a list of input files and
 * outputs them to a .h/.c pair so that you can reference them at compile/link
 * time.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/**
 * help text for the program
 */
static const char * help_text =
  "c-rez: a resource to c tool\n"
  "\n"
  "usage: c-rez -k <resource key> [-h <output.h>] "
  "[-c <output.c>] [--text] <input_1> "
  "[[--text] <input_2>] [[--text] <input_n>]*\n"
  "\n"
  " -h <file.h>:       specifies the header output file. If omitted, only "
  "source gets generated.\n"
  " -c <file.c>:       specifies the source output file. If omitted, only "
  "header gets generated.\n"
  " -k <resource key>: specifies an key to identify this resource. It will be "
  "used in header guards and resource functions.\n"
  " --text:            appends an \\0 when processing the next <input> file. "
  "This helps when using its data as a string resource.\n"
  " <input>:           space separated list of files to read from. "
  "Declarations and definitions will be generated based on the file name.\n"
  "                   If --text is specified before the file name, an '\\0' "
  "will be appended after processing.\n";

/**
 * parsed options for the program execution
 */
struct crez_opts {
  const char * key;
  const char * h_output;
  const char * c_output;
  const char * files[1024];
  unsigned file_count;
};

/**
 * the crez_arg struct helps process the program arguments. you can see it as an
 * argument iterator. see arg_construct, arg_next and arg_finished.
 */
struct crez_arg {
  int argc;
  const char ** argv;
  int i;
};

/**
 * a resource node to later transform in a series of switch/cases
 * to locate resources by name
 */
struct crez_node {
  char * symbol;
  struct crez_node * chilren[256];
};

/**
 * parses options or exit if problematic. fills in opts with options and files.
 *
 * @param opts the options struct to fill
 * @param argc count of arguments in argv
 * @param argv argument array
 * @return 1 if arguments were parsed, or 0 if not.
 */
int opts_parse_or_exit(struct crez_opts * opts, int argc, const char * argv[]);

/**
 * prints help and exists the program, optionally printing an error message
 * before exiting.
 *
 * @param error the error message to print. can be NULL.
 * @return 0
 */
int print_help_and_exit(const char * error);

/**
 * constructs an crez_arg type
 *
 * @param arg the crez_arg instance to construct
 * @param argc count of arguments in argv
 * @param argv the argument array
 */
void arg_construct(struct crez_arg * arg, int argc, const char * argv[]);

/**
 * returns the next argument to be processed
 *
 * @param arg the crez_arg instance to use
 * @return pointer to the next argument
 */
const char * arg_next(struct crez_arg * arg);

/**
 * tells if all arguments in arg were processed
 *
 * @param arg the crez_arg instance to use
 * @return 1 if finished, 0 if not.
 */
int arg_finished(struct crez_arg * arg);

/**
 * creates a C-compatible identifier by replacing non-ascii characters with _
 *
 * @param input the input text
 * @param prefix the prefix
 * @return a pointer to a null-terminated char array that must be free'd by the
 *         user
 */
char * make_identifier(const char input[], const char prefix[]);

/**
 * allocates/creates a node with an specified symbol.
 * @param symbol the symbol to add. might be NULL
 * @return a node instance that needs to be destroyed with node_destroy
 */
struct crez_node * node_create(const char * symbol);

/**
 * destroys a node and all its children node with it.
 * @param node the node to destroy and free
 */
void node_destroy(struct crez_node * node);

/**
 * adds a new symbol to the tree, creating intermediary nodes if needed.
 * @param node the root node
 * @param name the name to parse
 * @param symbol the symbol to add
 * @returns 1 if the symbol was added or 0 if it existed already
 * @example node_add_symbol(root, "assets/sprites.png", "assets_sprites_png");
 */
int node_add_symbol(struct crez_node * node, const char * name,
                     const char * symbol);

/**
 * creates and write .h/.c files based on opts,
 * also creates the symbol index and writes it as a series of switch/cases
 * for the locate function.
 *
 * @param opts the options for the invocation
 */
void write_files(struct crez_opts * opts);

/**
 * writes the content of file `filename` to h_file and c_file, prefixing names
 * by `prefix`
 *
 * @param file_name the file to open, read and write to h_file and c_file
 * @param identifier the identifier to add
 * @param h_file the header file to write declarations
 * @param c_file the c file to write definitions
 * @param is_text if an additional `\0` should be appended to the data
 */
void write_file(const char * file_name, const char * identifier, FILE * h_file,
                FILE * c_file, int is_text);

/**
 * writes a byte with padding and indentation based on `current_column`
 *
 * @param file the file pointer to write to
 * @param byte the byte to write
 * @param current_column used to do formatting and comma placement
 * @return the new current_column that should be passed in the next invocation
 *         of this function
 */
size_t write_byte(FILE * file, unsigned char byte, size_t current_column);

/**
 * writes to `file` a declaration of the struct `c_rez_resource`
 *
 * @param file the file pointer to write to
 */
void write_resource_struct_declaration(FILE * file);

/**
 * writes the `extern "C" {` guards if __cplusplus is defined
 *
 * @param file the file to write to
 */
void write_cplusplus_extern_guard_opening(FILE * file);

/**
 * writes the closing brace of the `extern "C" {` guard
 *
 * @param file the file to write to
 */
void write_cplusplus_extern_guard_closing(FILE * file);

/**
 * writes the opening of the include guards based on identifier
 *
 * @param file the file to write to
 * @param identifier identifier to use in #ifndefs
 */
void write_include_guard_opening(FILE * file, const char * identifier);

/**
 * writes the closing of the include guards based on identifier
 *
 * @param file the file to write to
 * @param identifier identifier to use in #ifndefs
 */
void write_include_guard_closing(FILE * file, const char * identifier);

/**
 * writes the resource_locate function with switch-cases
 *
 * @param h_file the header file to write to
 * @param c_file the source file to write to
 * @param key the key of this rez group
 * @param root the root of the nodes
 */
void write_locate_function(FILE * h_file, FILE * c_file, const char * key,
                           struct crez_node * root);


/**
 * writes a node and its children recursively
 *
 * @param c_file the file to write
 * @param node the node to write
 * @param level which level to write the switch/case
 */
void write_node(FILE * c_file, struct crez_node * node, int level);

/* main function ------------------------------------------------------------ */

int main(int argc, const char * argv[]) {
  struct crez_opts opts;
  int ok = 0;
  ok = opts_parse_or_exit(&opts, argc, argv);
  if (!ok) return 1;

  write_files(&opts);
  return 0;
}

/* arguments and opts parser ------------------------------------------------ */

int opts_parse_or_exit(struct crez_opts * opts,
                       int argc,
                       const char * argv[]) {
  struct crez_arg state;
  const char * opt;

  arg_construct(&state, argc, argv);
  arg_next(&state);
  opts->c_output = opts->h_output = (void *)0;
  opts->file_count = 0;
  opts->key = (void *)0;

  if (arg_finished(&state)) return print_help_and_exit((void *)0);

  while (!arg_finished(&state)) {
    opt = arg_next(&state);
    switch (opt[0]) {
      case '-':
        switch (opt[1]) {
          case 'c':
            opts->c_output = arg_next(&state);
            if (!opts->c_output)
              return print_help_and_exit("-c: no file specified.");
            break;
          case 'h':
            opts->h_output = arg_next(&state);
            if (!opts->h_output)
              return print_help_and_exit("-h: no file specified.");
            break;
          case 'k':
            opts->key = arg_next(&state);
            if (!opts->key)
              return print_help_and_exit("-k: no resource key specified.");
            break;
          case '-':
            opts->files[opts->file_count++] = opt;
            break;
          case '\0':
          default:
            return print_help_and_exit("unknown option.");
        }
        break;
      default:
        opts->files[opts->file_count++] = opt;
        break;
    }
  }

  if (!opts->c_output && !opts->h_output)
    return print_help_and_exit("no header nor source output specified "
                               "(use -h and/or -c).");

  if (!opts->key)
    return print_help_and_exit("no resource key specified (use -k).");

  return 1;
}

void arg_construct(struct crez_arg * arg, int argc, const char ** argv) {
  arg->argc = argc;
  arg->argv = argv;
  arg->i = 0;
}

const char * arg_next(struct crez_arg * arg) {
  if (arg->i >= arg->argc) return (void *)0;
  return arg->argv[arg->i++];
}

int print_help_and_exit(const char * error) {
  if (error) printf("\n%s\n\n%s", error, help_text);
  else printf("%s\n", help_text);
  exit(1);
  return 0;
}

int arg_finished(struct crez_arg * arg) {
  return arg->i >= arg->argc;
}

/* identifier maker ---------------------------------------------- */

char * make_identifier(const char input[], const char prefix[]) {
  size_t input_len = strlen(input),
    prefix_len = strlen(prefix),
    identifier_size = input_len + prefix_len + sizeof '_' + 1;
  char * identifier = calloc(identifier_size, sizeof(char));
  size_t src_i = 0, dst_i = 0;

  /* copies prefix */
  for (dst_i = 0, src_i = 0; src_i < prefix_len; src_i++, dst_i++) {
    identifier[dst_i] = isalnum(prefix[src_i]) ? prefix[src_i] : (char) '_';
  }

  identifier[dst_i++] = '_';

  /* copies input */
  for (src_i = 0; src_i < input_len; src_i++, dst_i++) {
    identifier[dst_i] = isalnum(input[src_i]) ? input[src_i] : (char) '_';
  }
  identifier[identifier_size - 1] = 0;
  
  return identifier;
}

/* writers ------------------------------------------------------------------ */

void write_files(struct crez_opts * opts) {
  unsigned i = 0;
  FILE * h_file, * c_file;
  c_file = h_file = (void *)0;
  char * h_identifier = (void *)0, * res_identifier = (void *)0;
  int wants_text = 0;
  struct crez_node * root = node_create((void *)0);

  if (opts->file_count == 0) {
    print_help_and_exit("no input files specified.");
    return;
  }

  if (opts->h_output) h_file = fopen(opts->h_output, "w+");
  if (opts->c_output) c_file = fopen(opts->c_output, "w+");

  if (!c_file && opts->c_output) {
    fprintf(stderr, "Cannot open output file %s: %s\n", opts->c_output,
            strerror(errno));
    exit(1);
  }

  if (!h_file && opts->h_output) {
    fprintf(stderr, "Cannot open output file %s: %s\n", opts->h_output,
            strerror(errno));
    exit(1);
  }

  if (h_file) {
    h_identifier = make_identifier(opts->h_output, opts->key);
    write_include_guard_opening(h_file, h_identifier);
    fprintf(h_file, "\n");

    write_cplusplus_extern_guard_opening(h_file);
    fprintf(h_file, "\n");

    write_resource_struct_declaration(h_file);
    fprintf(h_file, "\n");
  }

  if (c_file) {
    write_cplusplus_extern_guard_opening(c_file);
    fprintf(c_file, "\n");

    write_resource_struct_declaration(c_file);
    fprintf(c_file, "\n");
  }

  for (i = 0; i < opts->file_count; i++) {
    if (strcmp(opts->files[i], "--text") == 0) {
      wants_text = 1;
      continue;
    }

    res_identifier = make_identifier(opts->files[i], opts->key);

    /* do not write again if symbol exists already */
    if (node_add_symbol(root, opts->files[i], res_identifier)) {
      write_file(opts->files[i], res_identifier, h_file, c_file, wants_text);
    }
    fprintf(c_file, "\n");
    free(res_identifier);
    wants_text = 0;
  }

  write_locate_function(h_file, c_file, opts->key, root);

  if (h_file) {
    fprintf(h_file, "\n");

    write_cplusplus_extern_guard_closing(h_file);
    fprintf(h_file, "\n");

    write_include_guard_closing(h_file, h_identifier);
    fprintf(h_file, "\n");
  }

  node_destroy(root);
  free(h_identifier);
  fclose(c_file);
  fclose(h_file);
}

void write_file(const char * file_name, const char * identifier, FILE * h_file,
                FILE * c_file, int is_text) {
  FILE * in_file = fopen(file_name, "rb");
  unsigned char c = 0;
  size_t n = 0, column = 0, total = 0;
  if (!h_file && !c_file) { return; }
  if (!in_file) {
    fprintf(stderr, "Cannot open input file %s: %s\n", file_name,
            strerror(errno));
    return;
  }

  /* prints struct declaration */
  if (h_file) {
    fprintf(h_file, "extern c_rez_resource const %s;\n", identifier);
  }

  if (c_file) {
    /* prints data */
    fprintf(c_file, "unsigned char const %s_data[] = {\n", identifier);
    n = fread(&c, sizeof(c), 1, in_file);
    total = n;
    while (n) {
      column = write_byte(c_file, c, column);
      n = fread(&c, sizeof(c), 1, in_file);
      total += n;
    }

    if (is_text) {
      write_byte(c_file, 0, column);
      total++;
    }
    fprintf(c_file, "\n};\n");

    /* prints struct definition */
    fprintf(c_file, "struct c_rez_resource const %s = { %s_data, %lu };\n",
            identifier, identifier, total);
  }
}

void write_include_guard_opening(FILE * file, const char * identifier) {
  fprintf(file,
          "#ifndef c_rez_%s\n"
          "#define c_rez_%s\n",
          identifier, identifier);
}

void write_include_guard_closing(FILE * file, const char * identifier) {
  fprintf(file, "#endif /* c_rez_%s */\n", identifier);
}

void write_cplusplus_extern_guard_opening(FILE * file) {
  fprintf(file, "#ifdef __cplusplus\n"
                "extern \"C\" {\n"
                "#endif\n");
}

void write_cplusplus_extern_guard_closing(FILE * file) {
  fprintf(file, "#ifdef __cplusplus\n"
                "}\n"
                "#endif\n");
}


void write_space(FILE * file, int amount) {
  while (amount--) putc(' ', file);
}

void write_resource_struct_declaration(FILE * file) {
  fprintf(file,
          "#ifndef c_rez_resource_struct\n"
          "#define c_rez_resource_struct\n"
          "typedef struct c_rez_resource {\n"
          "  unsigned char const * const data;\n"
          "  unsigned int const length;\n"
          "} c_rez_resource;\n"
          "#endif /* c_rez_resource_struct */\n"
  );
}

size_t
write_byte(FILE * file, unsigned char byte, size_t current_column) {
  if (current_column > 0) {
    fprintf(file, ",");
  }
  if (current_column == 15) {
    fprintf(file, "\n");
    current_column = 0;
  }
  if (current_column == 0) { fprintf(file, "  "); }
  fprintf(file, " %3d", byte);
  return current_column + 1;
}

void write_locate_function(FILE * h_file, FILE * c_file, const char * key,
                           struct crez_node * root) {
  if (h_file) {
    fprintf(h_file, "struct c_rez_resource const * "
                    "c_rez_locate_%s(const char name[]);", key);
  }
  if (c_file) {
    fprintf(c_file, "struct c_rez_resource const * "
                    "c_rez_locate_%s(const char name[]) {\n", key);
    write_node(c_file, root, 0);
    fprintf(c_file, "}\n");
  }
}

void write_node(FILE * c_file, struct crez_node * node, int level) {
  int i = 0, indent = level * 2 + 2;
  const char * symbol = node->chilren[0] && node->chilren[0]->symbol ? node->chilren[0]->symbol : (void *)0;
  write_space(c_file, indent);
  fprintf(c_file, "switch (name[%d]) {\n", level);

  /* writes the 0th case, the symbol itself */
  write_space(c_file, indent);
  fprintf(c_file, "  case 0: return%s%s;\n", symbol ? " &" : " ", symbol ? symbol : "(void *) 0");

  /* writes children nodes */
  for (i = 1; i < 256; i++) {
    if (node->chilren[i]) {
      write_space(c_file, indent);
      fprintf(c_file, "  case '%c':\n", i);
      write_node(c_file, node->chilren[i], level + 1);
    }
  }

  /* writes default case */
  write_space(c_file, indent);
  fprintf(c_file, "  default: return (void *) 0;\n");

  /* writes end of swich/case */
  write_space(c_file, indent);
  fprintf(c_file, "}\n");
}

/* tree builders ------------------------------------------------------------ */

struct crez_node * node_create(const char * symbol) {
  struct crez_node * node = calloc(1, sizeof(*node));
  size_t len = 0, i = 0;
  if (symbol) {
    len = strlen(symbol);
    node->symbol = calloc(len +1, sizeof(symbol));
    for (i = 0; i < len; i++) node->symbol[i] = symbol[i];
  }
  return node;
}

void node_destroy(struct crez_node * node) {
  int i = 0;
  if (node->symbol) free(node->symbol);
  for (i = 0; i < 256; i++) {
    if (node->chilren[0]) node_destroy(node->chilren[0]);
  }
  free(node);
}

int node_add_symbol(struct crez_node * node, const char * name,
  const char * symbol) {
  size_t name_len = strlen(name), i = 0;
  char key;

  /* this will add all nodes up until what we need */
  for (key = name[0], i = 0; i < name_len; i++, key = name[i]) {
    if (!node->chilren[key]) {
      node->chilren[key] = node_create((void *)0);
    }
    node = node->chilren[key];
  }

  /* only add a new node if old wasn't found */
  if (node->chilren[0] == (void *)0) {
    node->chilren[0] = node_create(symbol);
    return 1;
  }
  return 0;
}
