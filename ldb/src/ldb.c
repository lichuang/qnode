#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ldb.h"

#define LDB_MAX_INPUT 200
#define LDB_MAX_PARAM 5

typedef struct input_t {
  char    buffer[LDB_MAX_INPUT][LDB_MAX_PARAM];
  int     num;
} input_t;

static void single_step(ldb_t *ldb, int step);
static void enable_line_hook(lua_State *state, int enable);
static void line_hook(lua_State *state, lua_Debug *ar);
//static void func_hook(lua_State *state, lua_Debug *ar);
static void output( const char * format, ... );
static int  get_input(char *buff, int size);
static void set_prompt();
static int  split_input(const char *buff, input_t *input);
static int  search_local_var(lua_State *state, lua_Debug *ar,
                             const char* var);

static int  search_global_var(lua_State *state, lua_Debug *ar,
                              const char* var);
static void print_var(lua_State *state, int si, int depth);
static void print_table_var(lua_State *state, int si, int depth);
static void print_string_var(lua_State *state, int si, int depth);
static void dump_stack(lua_State *state, int depth, int verbose); 

static int help_handler(lua_State *state,  lua_Debug *ar, input_t *input);
static int quit_handler(lua_State *state,  lua_Debug *ar, input_t *input);
static int print_handler(lua_State *state, lua_Debug *ar, input_t *input);
static int backtrace_handler(lua_State *state, lua_Debug *ar, input_t *input);

typedef int (*handler_t)(lua_State *state, lua_Debug *ar, input_t *input);

typedef struct ldb_command_t {
  const char* name;
  handler_t   handler;
} ldb_command_t;

ldb_command_t commands[] = {
  {"help", &help_handler},
  {"h",    &help_handler},
  {"quit", &quit_handler},
  {"q",    &quit_handler},
  {"p",    &print_handler},
  {"bt",   &backtrace_handler},
  {NULL,    NULL},
};

ldb_t*
ldb_new() {
  ldb_t *ldb;

  ldb = (ldb_t*)malloc(sizeof(ldb_t));
  if (ldb == NULL) {
    return NULL;
  }
  ldb->step = 0;
  ldb->call_depth = 0;

  return ldb;
}

void
ldb_destroy(ldb_t *ldb) {
  if (ldb) {
    free(ldb);
  }
}

void
ldb_attach(ldb_t *ldb, lua_State *state) {
  lua_pushlightuserdata(state, state);
  lua_pushlightuserdata(state, ldb);
  lua_settable(state, LUA_REGISTRYINDEX);
}

void
ldb_step_in(lua_State *state, int step) {
  ldb_t *ldb;

  lua_pushlightuserdata(state, state);
  lua_gettable(state, LUA_REGISTRYINDEX);
  ldb = (ldb_t*)lua_touserdata(state, -1);
  if (ldb == NULL) {
    return;
  }
  ldb->state = state;
  if (ldb->step == 0) {
    single_step(ldb, 1);
  }

  ldb->call_depth = -1;
}

static void
single_step(ldb_t *ldb, int step) {
  if (step) {
    enable_line_hook(ldb->state, 1);
  }

  ldb->step = step;
}

static void
enable_line_hook(lua_State *state, int enable) {
  int mask;
  
  mask = lua_gethookmask(state);
  if (enable) {
    lua_sethook(state, line_hook, mask | LUA_MASKLINE, 0); 
  } else {
    lua_sethook(state, line_hook, mask & ~LUA_MASKLINE, 0); 
  }
}

static int
get_input(char *buff, int size) {
  int len = read(STDIN_FILENO, buff, size);
  if(len > 0) { 
    buff[len - 1] = '\0';
    return len - 1;
  }   
  return -1;
}

static int
split_input(const char *buff, input_t *input) {
   const char *p, *save;
   int i;

   save = p = buff;
   save = NULL;
   i = 0;
   while (p && *p) {
     if (*p == '\n') {
       break;
     }

     if (isspace(*p)) {
       if (save) {
         if (i > LDB_MAX_PARAM) {
           output("param %s more than %d", buff, LDB_MAX_PARAM);
           return -1;
         }
         strncpy(input->buffer[i++], save, p - save);
         save = NULL;
       }
     } else {
       if (save == NULL) {
         save = p;
       }
     }

     ++p;
   }
   if (i > LDB_MAX_PARAM) {
     output("param %s more than %d", buff, LDB_MAX_PARAM);
     return -1;
   }
   strcpy(input->buffer[i++], save);
   input->num = i;

   /*
   output("input: ");
   int j = 0;
   for (j = 0; j < i; ++j) {
     output("%s +", input[j]);
   }
   output("\n");
   */

   return 0;
}

static void
line_hook(lua_State *state, lua_Debug *ar) {
  input_t input;;

  if( !lua_getstack(state, 0, ar )) { 
    output("[LUA_DEBUG]lua_getstack fail\n");
    return;
  }

  if( lua_getinfo(state, "lnS", ar ) ) {
    set_prompt();
  } else {
    output("[LUA_DEBUG]lua_getinfo fail\n");
    return;
  }

   char buff[LDB_MAX_INPUT];
   int ret, i;
   while (get_input(&buff[0], sizeof(buff)) > 0) {
     if (split_input(&(buff[0]), &input) < 0) {
       set_prompt();
       continue;
     }

     ret = 0;
     for (i = 0; commands[i].handler != NULL; ++i) {
       if (strcmp(input.buffer[0], commands[i].name) == 0) {
         ret = (*commands[i].handler)(state, ar, &input);
         break;
       }
     }
     if (commands[i].name == NULL) {
       output("bad command: %s, output h for help\n", buff);
     }
     //input.clear();
     if (ret < 0) {
       break;
     }
     set_prompt();
   }
}

/*
static void
func_hook(lua_State *state, lua_Debug *ar) {
}
*/

static void
set_prompt() {   
  output("(ldb) ");
}  

static void
output(const char * format, ... ) {
  va_list arg;

  va_start(arg, format);
  vfprintf(stdout, format, arg);
  va_end(arg);

  fflush(stdout);
}

static int
help_handler(lua_State *state, lua_Debug *ar, input_t *input) {
  output("Lua debugger written by Lichuang(2013)\n"
         "cmd:\n"
         "\th(help): print help info\n"
         "\tq(quit): quit ldb\n"
         "\tp <varname>: print var value\n"
         "\tbt: print backtrace info\n"
         );

  return 0;
}

static int
quit_handler(lua_State *state, lua_Debug *ar, input_t *input) {
  //output( "Continue...\n" );
  enable_line_hook(state, 0);

  return -1;
} 

static int
print_handler(lua_State *state, lua_Debug *ar, input_t *input) {
  if (input->num < 2) {
    output("usage: p <varname>\n");
    return 0;
  }

  if (search_local_var(state, ar, input->buffer[1])) {
    output("local %s =", input->buffer[1]);
    print_var(state, -1, -1);
    lua_pop(state, 1);
    output("\n");
  } else if (search_global_var(state, ar, input->buffer[1])) {
    output("global %s =", input->buffer[1]);
    print_var(state, -1, -1);
    lua_pop(state, 1);
    output("\n");
  } else {
    output("not found var %s\n",  input->buffer[1]);
  }

  return 0;
}

static int
search_local_var(lua_State *state, lua_Debug *ar, const char* var) {
  int         i;
  const char *name;

  for(i = 1; (name = lua_getlocal(state, ar, i)) != NULL; i++) {
    if(strcmp(var,name) == 0) {
      return i;
    }    
    // not match, pop out the var's value
    lua_pop(state, 1); 
  }
  return 0;
}

static int
search_global_var(lua_State *state, lua_Debug *ar, const char* var) {
  lua_getglobal(state, var);

  if(lua_type(state, -1 ) == LUA_TNIL) {
    lua_pop(state, 1);
    return 0;
  }   

  return 1;
}

static void
print_table_var(lua_State *state, int si, int depth) {
  int pos_si = si > 0 ? si : (si - 1);
  output("{");
  int top = lua_gettop(state);
  lua_pushnil(state);
  int empty = 1;
  while(lua_next(state, pos_si ) !=0) {
    if(empty) {
      output("\n");
      empty = 0;
    }

    int i;
    for(i = 0; i < depth; i++) {
      output("\t");
    }

    output( "[" );
    print_var(state, -2, -1);
    output( "] = " );
    if(depth > 5) {
      output("{...}");
    } else {
      print_var(state, -1, depth + 1);
    }
    lua_pop(state, 1);
    output(",\n");
  }

  if (empty) {
    output(" }");
  } else {
    int i;
    for (i = 0; i < depth - 1; i++) {
      output("\t");
    }
    output("}");
  }
  lua_settop(state, top);
}

static void 
print_string_var(lua_State *state, int si, int depth) {
  output( "\"" );

  const char * val = lua_tostring(state, si);
  int vallen = lua_strlen(state, si);
  int i;
  const char spchar[] = "\"\t\n\r";
  for(i = 0; i < vallen; ) {
    if(val[i] == 0) {
      output("\\000");
      ++i;
    } else if (val[i] == '"') {
      output("\\\"");
      ++i;
    } else if(val[i] == '\\') {
      output("\\\\");
      ++i;
    } else if(val[i] == '\t') {
      output("\\t");
      ++i;
    } else if(val[i] == '\n') {
      output("\\n");
      ++i;
    } else if(val[i] == '\r') {
      output("\\r");
      ++i;
    } else {
      int splen = strcspn(val + i, spchar);

      output("%.*s", splen, val+i);
      i += splen;
    }
  }
  output("\"");
}

static void
dump_stack(lua_State *state, int depth, int verbose) {
  lua_Debug ldb; 
  int i;
  const char *name, *filename;
  /*
  int addr_len;
  char fn[4096];
  */

  for(i = depth; lua_getstack(state, i, &ldb) == 1; i++) {
    lua_getinfo(state, "Slnu", &ldb);
    name = ldb.name;
    if( !name ) {
      name = "";
    }    
    filename = ldb.source;

    output("#%d: %s:'%s', '%s' line %d\n",
           i + 1 - depth, ldb.what, name,
           filename, ldb.currentline );
    /*
    addr_len = strlen(strlcpy( fn, search_path_, 4096)); 
    strlcpy(fn + addr_len, filename + 3, 4096-addr_len ); // @./
    */
    /*
    if(verbose) {
      if( ldb.source[0]=='@' && ldb.currentline!=-1 ) {
        const char * line = src_manager_->load_file_line( fn, ldb.currentline-1 );
        if( line ) {
          print( "%s\n", line );
        } else {
          print( "[no source available]\n" );
        }    
      } else {
        print( "[no source available]\n" );
      }
    }
    */
  }
}

static void
print_var(lua_State *state, int si, int depth) {
  switch(lua_type(state, si)) {
  case LUA_TNIL:
    output("(nil)");
    break;

  case LUA_TNUMBER:
    output("%f", lua_tonumber(state, si));
    break;

  case LUA_TBOOLEAN:
    output("%s", lua_toboolean(state, si) ? "true":"false");
    break;

  case LUA_TFUNCTION:
    {
      lua_CFunction func = lua_tocfunction(state, si);
      if( func != NULL ) {
        output("(C function)0x%p", func);
      } else {
        output("(function)");
      }
    }
    break;

  case LUA_TUSERDATA:
    output("(user data)0x%p", lua_touserdata(state, si));
    break;

  case LUA_TSTRING:
    print_string_var(state, si, depth);
    break;

  case LUA_TTABLE:
    print_table_var(state, si, depth);
    break;

  default:
    break;
  }
}

static int
backtrace_handler(lua_State *state, lua_Debug *ar, input_t *input) {
  dump_stack(state, 0, 0);

  return 0;
}
