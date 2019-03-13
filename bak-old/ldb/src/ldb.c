#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "ldb.h"
#include "ldb_util.h"

#define LDB_MAX_INPUT 200
#define LDB_MAX_PARAM 5

static const char* lua_tag = "__ldb_tag";

typedef struct input_t {
  char    buffer[LDB_MAX_PARAM][LDB_MAX_INPUT];
  int     num;
} input_t;

static char *prompt = "(ldb)";

static void single_step(ldb_t *ldb, int step);
static void enable_line_hook(lua_State *state, int enable);
static void enable_func_hook(lua_State *state, ldb_t *ldb, int enable);
static void all_hook(lua_State *state, lua_Debug *ar);
//static void func_hook(lua_State *state, lua_Debug *ar);
static int  get_input(char **buff);
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
static int  get_calldepth(lua_State *state);
static void on_event(int bp, ldb_t *ldb, lua_State *state, lua_Debug *ar);
static void step_in(lua_State *state, ldb_t *ldb, int depth, lua_Debug *ar);
static void print_current_line(ldb_t *ldb, lua_Debug *ar);

static void handle_line_break(lua_State *state, ldb_t *ldb,
                              lua_Debug *ar, input_t *input,
                              char *dv);
static void handle_func_break(lua_State *state, ldb_t *ldb,
                              lua_Debug *ar, input_t *input);
static int  add_line_break_point(ldb_t *ldb, const char *file, int line);
static int  add_func_break_point(ldb_t *ldb, const char *func, const char *type);
//static void delete_break_point(ldb_t *ldb, int index);
static int  search_break_point(ldb_t *ldb, lua_Debug *ar);
static int  enable_break_point(ldb_t *ldb, input_t *input, int enable);

static int help_handler(lua_State *state,  ldb_t *ldb,
                        lua_Debug *ar, input_t *input);
static int print_handler(lua_State *state, ldb_t *ldb,
                         lua_Debug *ar, input_t *input);
static int backtrace_handler(lua_State *state, ldb_t *ldb,
                             lua_Debug *ar, input_t *input);
static int list_handler(lua_State *state, ldb_t *ldb,
                        lua_Debug *ar, input_t *input);
static int step_handler(lua_State *state, ldb_t *ldb,
                        lua_Debug *ar, input_t *input);
static int next_handler(lua_State *state, ldb_t *ldb,
                        lua_Debug *ar, input_t *input);
static int break_handler(lua_State *state, ldb_t *ldb,
                         lua_Debug *ar, input_t *input);
static int continue_handler(lua_State *state, ldb_t *ldb,
                            lua_Debug *ar, input_t *input);
static int info_handler(lua_State *state, ldb_t *ldb,
                        lua_Debug *ar, input_t *input);
static int disable_handler(lua_State *state, ldb_t *ldb,
                           lua_Debug *ar, input_t *input);
static int enable_handler(lua_State *state, ldb_t *ldb,
                          lua_Debug *ar, input_t *input);
static int delete_handler(lua_State *state, ldb_t *ldb,
                          lua_Debug *ar, input_t *input);
static int reload_handler(lua_State *state, ldb_t *ldb,
                          lua_Debug *ar, input_t *input);

typedef int (*handler_pt)(lua_State *state, ldb_t *ldb,
                          lua_Debug *ar, input_t *input);

typedef struct ldb_command_t {
  const char* name;
  const char* short_name;
  const char* help;
  handler_pt  handler;
} ldb_command_t;

ldb_command_t commands[] = {
  {
    "help",
    "h",
    ": print help info",
    &help_handler
  },

  {
    "print",
    "p",
    "<varname> : print var value",
    &print_handler
  },

  {
    "backtrace",
    "bt",
    ": print backtrace info",
    &backtrace_handler
  },

  {
    "list",
    "l",
    ": list file source",
    &list_handler
  },

  {
    "step",
    "s",
    ": one instruction exactly",
    &step_handler
  },

  {
    "next",
    "n",
    ":next line",
    &next_handler
  },

  {
    "break",
    "b",
    "[function|filename:line] : break at function or line in a file",
    &break_handler
  },

  {
    "disable",
    "dis",
    "breakpoint : disable a breakpoint",
    &disable_handler
  },

  {
    "enable",
    "en",
    "breakpoint : enable a breakpoint",
    &enable_handler
  },

  {
    "delete",
    "del",
    "breakpoint : delete a breakpoint",
    &delete_handler
  },

  {
    "info",
    "i",
    ":show all break info",
    &info_handler
  },

  {
    "continue",
    "c",
    ": continue execute when hit a break point",
    &continue_handler
  },

  {
    "reload",
    "r",
    ": reload a lua script",
    &reload_handler
  },

  {NULL,        NULL, NULL,                                 NULL},
};

ldb_t*
ldb_new(lua_State *state, ldb_reload_pt reload) {
  int    i;
  ldb_t *ldb;

  lua_gettable(state, LUA_REGISTRYINDEX);
  ldb = (ldb_t*)lua_touserdata(state, -1);
  /* already exist? */
  if (ldb) {
    return ldb;
  }

  ldb = (ldb_t*)malloc(sizeof(ldb_t));
  if (ldb == NULL) {
    return NULL;
  }
  ldb->step = 0;
  ldb->first = 1;
  ldb->call_depth = 0;
  ldb->state = state;
  ldb->reload = reload;

  lua_pushstring(state, lua_tag);
  lua_pushlightuserdata(state, ldb);
  lua_settable(state, LUA_REGISTRYINDEX);

  for (i = 0; i < MAX_FILE_BUCKET; ++i) {
    ldb->files[i] = NULL;
  }
  for (i = 0; i < MAX_BREAKPOINT; ++i) {
    ldb->bkpoints[i].available = 1;
    ldb->bkpoints[i].file = NULL;
    ldb->bkpoints[i].func = NULL;
  }
  ldb->bknum = 0;
  return ldb;
}

void
ldb_free(ldb_t *ldb) {
  int         i;
  ldb_file_t *file, *next;
  lua_State  *state;
  ldb_breakpoint_t *bkpoint;

  state = ldb->state;
  for (i = 0; i < MAX_FILE_BUCKET; ++i) {
    file = ldb->files[i];
    while (file) {
      next = file->next;
      ldb_file_free(file);
      file = next;
    }
  }
  for (i = 0; i < MAX_BREAKPOINT; ++i) {
    bkpoint = &(ldb->bkpoints[i]);
    if (bkpoint->file) {
      free(bkpoint->file);
    }
    if (bkpoint->func) {
      free(bkpoint->file);
    }
  }
  free(ldb);
  lua_pushstring(state, lua_tag);
  lua_pushlightuserdata(state, NULL);
  lua_settable(state, LUA_REGISTRYINDEX);
}

void
ldb_break(lua_State *state) {
  ldb_t *ldb;

  lua_pushstring(state, lua_tag);
  lua_gettable(state, LUA_REGISTRYINDEX);
  ldb = (ldb_t*)lua_touserdata(state, -1);
  if (ldb == NULL) {
    return;
  }
  ldb->state = state;
  if (ldb->step == 0) {
    single_step(ldb, 1);
  }

  ldb->step       = 1;
  ldb->first      = 1;
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
    lua_sethook(state, all_hook, mask | LUA_MASKLINE, 0); 
  } else {
    lua_sethook(state, all_hook, mask & ~LUA_MASKLINE, 0); 
  }
}

static void
enable_func_hook(lua_State *state, ldb_t *ldb, int enable) {
  int mask;
  
  mask = lua_gethookmask(state);
  if (enable) {
    lua_sethook(state, all_hook, mask | LUA_MASKCALL, 0); 
    ldb->step = 0;
  } else {
    lua_sethook(state, all_hook, mask & ~LUA_MASKCALL, 0); 
  }
}

static int
get_input(char **buff) {
  *buff = readline(prompt);
  int len = strlen(*buff);
  if(len > 0) { 
    add_history(*buff);
    return len;
  }   
  return len;
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
           ldb_output("param %s more than %d", buff, LDB_MAX_PARAM);
           return -1;
         }
         strncpy(input->buffer[i], save, p - save);
         input->buffer[i++][p - save] = '\0';
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
     ldb_output("param %s more than %d", buff, LDB_MAX_PARAM);
     return -1;
   }
   if (save) {
     strcpy(input->buffer[i++], save);
   }
   input->num = i;

   /*
   ldb_output("input: ");
   int j = 0;
   for (j = 0; j < i; ++j) {
     ldb_output("%s +", input[j]);
   }
   ldb_output("\n");
   */

   return 0;
}

static void
all_hook(lua_State *state, lua_Debug *ar) {
  int    index;
  ldb_t  *ldb;

  if(!lua_getstack(state, 0, ar)) { 
    ldb_output("[LUA_DEBUG]lua_getstack fail\n");
    return;
  }

  lua_pushstring(state, lua_tag);
  lua_gettable(state, LUA_REGISTRYINDEX);
  ldb = (ldb_t*)lua_touserdata(state, -1);
  if (ldb == NULL) {
    return;
  }

  //if(lua_getinfo(state, "lnS", ar)) {
  if(lua_getinfo(state, "lnSu", ar)) {
    if (ldb->step) {
      if (ldb->first) {
        ldb->first = 0;
        ldb_output("Break at %s:%d\n", ar->source + 1, ar->currentline);
      }
      on_event(-1, ldb, state, ar);
      return;
    } else {
      index = search_break_point(ldb, ar);
      if (index < 0) {
        return;
      }
      on_event(index, ldb, state, ar);
      return;
    }
  } else {
    ldb_output("[LUA_DEBUG]lua_getinfo fail\n");
    return;
  }
}

static void
set_prompt() {   
  //ldb_output(prompt);
}  

static int
help_handler(lua_State *state, ldb_t *ldb,
             lua_Debug *ar, input_t *input) {
  int i;

  ldb_output("Lua debugger written by Lichuang(2013)\ncmd:\n");
  for (i = 0; commands[i].name != NULL; ++i) {
    if (commands[i].help) {
      ldb_output("\t%s(%s) %s\n", commands[i].name,commands[i].short_name,commands[i].help);
    }
  }
  return 0;
}

static int
print_handler(lua_State *state, ldb_t *ldb,
              lua_Debug *ar, input_t *input) {
  if (input->num < 2) {
    ldb_output("usage: p <varname>\n");
    return 0;
  }

  if (search_local_var(state, ar, input->buffer[1])) {
    ldb_output("local %s = ", input->buffer[1]);
    print_var(state, -1, -1);
    lua_pop(state, 1);
    ldb_output("\n");
  } else if (search_global_var(state, ar, input->buffer[1])) {
    ldb_output("global %s = ", input->buffer[1]);
    print_var(state, -1, -1);
    lua_pop(state, 1);
    ldb_output("\n");
  } else {
    ldb_output("not found var %s\n",  input->buffer[1]);
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

  if(lua_type(state, -1) == LUA_TNIL) {
    lua_pop(state, 1);
    return 0;
  }   

  return 1;
}

static void
print_table_var(lua_State *state, int si, int depth) {
  int pos_si = si > 0 ? si : (si - 1);
  ldb_output("{");
  int top = lua_gettop(state);
  lua_pushnil(state);
  int empty = 1;
  while(lua_next(state, pos_si ) !=0) {
    if(empty) {
      ldb_output("\n");
      empty = 0;
    }

    int i;
    for(i = 0; i < depth; i++) {
      ldb_output("\t");
    }

    ldb_output( "[" );
    print_var(state, -2, -1);
    ldb_output( "] = " );
    if(depth > 5) {
      ldb_output("{...}");
    } else {
      print_var(state, -1, depth + 1);
    }
    lua_pop(state, 1);
    ldb_output(",\n");
  }

  if (empty) {
    ldb_output(" }");
  } else {
    int i;
    for (i = 0; i < depth - 1; i++) {
      ldb_output("\t");
    }
    ldb_output("}");
  }
  lua_settop(state, top);
}

static void 
print_string_var(lua_State *state, int si, int depth) {
  ldb_output( "\"" );

  const char * val = lua_tostring(state, si);
  int vallen = lua_strlen(state, si);
  int i;
  const char spchar[] = "\"\t\n\r";
  for(i = 0; i < vallen; ) {
    if(val[i] == 0) {
      ldb_output("\\000");
      ++i;
    } else if (val[i] == '"') {
      ldb_output("\\\"");
      ++i;
    } else if(val[i] == '\\') {
      ldb_output("\\\\");
      ++i;
    } else if(val[i] == '\t') {
      ldb_output("\\t");
      ++i;
    } else if(val[i] == '\n') {
      ldb_output("\\n");
      ++i;
    } else if(val[i] == '\r') {
      ldb_output("\\r");
      ++i;
    } else {
      int splen = strcspn(val + i, spchar);

      ldb_output("%.*s", splen, val+i);
      i += splen;
    }
  }
  ldb_output("\"");
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
    if(!name) {
      name = "";
    }    
    filename = ldb.source;

    ldb_output("#%d: %s:'%s', '%s' line %d\n",
           i + 1 - depth, ldb.what, name,
           filename, ldb.currentline );
  }
}

static void
print_var(lua_State *state, int si, int depth) {
  int type;

  type = lua_type(state, si);
  switch(type) {
  case LUA_TNIL:
    ldb_output("(nil)");
    break;

  case LUA_TNUMBER:
    ldb_output("%f", lua_tonumber(state, si));
    break;

  case LUA_TBOOLEAN:
    ldb_output("%s", lua_toboolean(state, si) ? "true":"false");
    break;

  case LUA_TFUNCTION:
    {
      lua_CFunction func = lua_tocfunction(state, si);
      if( func != NULL ) {
        ldb_output("(C function)%p", func);
      } else {
        ldb_output("(function)");
      }
    }
    break;

  case LUA_TUSERDATA:
    ldb_output("(user data)%p", lua_touserdata(state, si));
    break;

  case LUA_TLIGHTUSERDATA:
    ldb_output("(light user data)%p", lua_touserdata(state, si));
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
backtrace_handler(lua_State *state, ldb_t *ldb,
                  lua_Debug *ar, input_t *input) {
  dump_stack(state, 0, 0);

  return 0;
}

static int
list_handler(lua_State *state, ldb_t *ldb,
             lua_Debug *ar, input_t *input) {
  ldb_file_t *file;
  int         i, j;

  /* ignore `@` char */
  file = ldb_file_load(ldb, ar->source + 1);
  if (file == NULL) {
    return 0;
  }

  i = ar->currentline - 5;
  if (i < 0) {
    i = 0;
  }
  for (; i < ar->currentline; ++i) {
    ldb_output("%s:%d\t%s", file->name, i + 1, file->lines[i]);
  }
  for (i = ar->currentline, j = 0; j < 6 && i < file->line; ++j, ++i) {
    ldb_output("%s:%d\t%s", file->name, i + 1, file->lines[i]);
  }
  return 0;
}

static int
get_calldepth(lua_State *state) {
  int i;
  lua_Debug ar;

  for (i = 0; lua_getstack(state, i + 1, &ar ) != 0; i++)
    ;
  return i;                                                                   
}  

static void
on_event(int bp, ldb_t *ldb, lua_State *state, lua_Debug *ar) {
  int depth;

  depth = get_calldepth(state);
  if (bp < 0 && ldb->call_depth != -1 && depth > ldb->call_depth) {
    /* next command, just return */
    return;
  }
  ldb->call_depth = depth;

  if (bp >= 0) {
    ldb_output("Breakpoint %d hit!\n", bp);
  }

  set_prompt();
  char *buff;
  input_t input;;
  int ret, i, len;
  while ((len = get_input(&buff)) >= 0) {
    if (len == 0) {
      free(buff);
      continue;
    }
    if (split_input(buff, &input) < 0) {
      set_prompt();
      free(buff);
      continue;
    }

    ret = 0;
    for (i = 0; commands[i].handler != NULL; ++i) {
      if (!strcmp(input.buffer[0], commands[i].short_name) ||
        !strcmp(input.buffer[0], commands[i].name)) {
        ldb->call_depth = get_calldepth(state);
        ret = (*commands[i].handler)(state, ldb, ar, &input);
        break;
      }
    }
    if (commands[i].name == NULL) {
      ldb_output("bad command: %s, ldb_output h for help\n", buff);
    }
    free(buff);
    if (ret < 0) {
      break;
    }
    set_prompt();
  }
}

static void
step_in(lua_State *state, ldb_t *ldb,
        int depth, lua_Debug *ar) {
  if (ldb->step != 1) {
    single_step(ldb, 1);
  }
  if (depth == -1) {
    ldb->call_depth = -1;
  }
  print_current_line(ldb, ar);
}

static void
print_current_line(ldb_t *ldb, lua_Debug *ar) {
  ldb_file_t *file;
  int i;

  /* ignore `@` char */
  file = ldb_file_load(ldb, ar->source + 1);
  if (file == NULL) {
    return;
  }
  if (ar->currentline >= file->line) {
    return;
  }
  i = ar->currentline;
  ldb_output("%d\t%s", i + 1, file->lines[i]);
}

static int
step_handler(lua_State *state, ldb_t *ldb,
             lua_Debug *ar, input_t *input) {
  step_in(state, ldb, -1, ar);

  return -1;
}

static int
next_handler(lua_State *state, ldb_t *ldb,
             lua_Debug *ar, input_t *input) {
  step_in(state, ldb, 0, ar);

  return -1;
}

#if 0
static void
delete_break_point(ldb_t *ldb, int index) {
  ldb_breakpoint_t *bkpoint;

  bkpoint = &(ldb->bkpoints[index]);
  if (bkpoint->file == NULL) {
    return;
  }
  free(bkpoint->file);
  bkpoint->file = NULL;
}
#endif

static int
search_break_point(ldb_t *ldb, lua_Debug *ar) {
  int i;
  ldb_breakpoint_t *bkpoint;

  /* search break point by file:line */
  for (i = 0; i < ldb->bknum; ++i) {
    bkpoint = &(ldb->bkpoints[i]);
    if (bkpoint->available) {
      continue;
    }
    if (bkpoint->active == 0) {
      continue;
    }
    if (bkpoint->file == NULL) {
      continue;
    }
    if (ar->currentline != bkpoint->line + 1) {
      continue;
    }
    if (strcmp(ar->source + 1, bkpoint->file)) {
      continue;
    }

    bkpoint->hit += 1;
    return i;
  }

  /* if not hook by func call, return */
  if (ar->event != LUA_HOOKCALL) {
    return -1;
  }
  if (ar->name == NULL) {
    return -1;
  }
  /* search break point by func */
  for (i = 0; i < ldb->bknum; ++i) {
    bkpoint = &(ldb->bkpoints[i]);
    if (bkpoint->available) {
      continue;
    }
    if (bkpoint->active == 0) {
      continue;
    }
    if (strcmp(ar->name, bkpoint->func)) {
      continue;
    }

    bkpoint->hit += 1;
    return i;
  }
  return -1;
}

static int
add_line_break_point(ldb_t *ldb, const char *file, int line) {
  int i;
  ldb_breakpoint_t *bkpoint;

  for (i = 0; i < MAX_BREAKPOINT; ++i) {
    if (ldb->bkpoints[i].available) {
      break;
    }
  }

  if (i == MAX_BREAKPOINT) {
    return -1;
  }
  bkpoint = &(ldb->bkpoints[i]);
  bkpoint->available = 0;
  bkpoint->file   = strdup(file);
  bkpoint->line   = line;
  bkpoint->active = 1;
  bkpoint->index  = i;
  bkpoint->hit    = 0;
  bkpoint->func   = NULL;
  ldb->bknum += 1;

  return i;
}

static int
add_func_break_point(ldb_t *ldb, const char *func, const char *type) {
  int i;
  ldb_breakpoint_t *bkpoint;

  for (i = 0; i < MAX_BREAKPOINT; ++i) {
    if (ldb->bkpoints[i].available) {
      break;
    }
  }

  if (i == MAX_BREAKPOINT) {
    return -1;
  }
  bkpoint = &(ldb->bkpoints[i]);
  bkpoint->available = 0;
  bkpoint->file   = NULL;
  bkpoint->func   = strdup(func);
  bkpoint->line   = -1;
  bkpoint->active = 1;
  bkpoint->index  = i;
  bkpoint->hit    = 0;
  bkpoint->type   = type;
  ldb->bknum += 1;

  return i;
}

static void
handle_line_break(lua_State *state, ldb_t *ldb,
                  lua_Debug *ar, input_t *input,
                  char *dv) {
  char name[200];
  int line, index;
  ldb_file_t *file;

  line = atoi(dv + 1);
  *dv = '\0';
  strcpy(name, input->buffer[1]);

  file = ldb_file_load(ldb, name);
  if (file == NULL) {
    return;
  }
  if (line > file->line) {
    ldb_output("line too long\n");
    return;
  }
  index = add_line_break_point(ldb, name, line - 1);
  if (index < 0) {
    return;
  }
  ldb_output("breakpoint %d is set at %s:%d\n",
             index, name, line);
  enable_line_hook(state, 1);
}

static void
handle_func_break(lua_State *state, ldb_t *ldb,
                  lua_Debug *ar, input_t *input) {
  int top, index, len, pos;
  char *p, *buffer;

  top = lua_gettop(state);
  index = LUA_GLOBALSINDEX;
  buffer = &(input->buffer[1][0]);
  len = strlen(buffer);
  p  = strchr(buffer, '.');
  while (p) {
    lua_pushlstring(state, buffer, p - buffer);
    lua_gettable(state, index);
    if(lua_isnil(state, -1)) {
      lua_settop(state, top);
      ldb_output("Table '%.*s' not found!\n", p - buffer, buffer); 
      return;
    }   
    index = lua_gettop(state); 
    buffer = p + 1;
    p = strchr(buffer, '.');
  }

  pos = buffer - &(input->buffer[1][0]);
  lua_pushlstring(state, buffer, len - pos);
  lua_gettable(state, index);
  if(lua_isnil(state, -1 ) ) {
    ldb_output("Function '%.*s' not found!\n", len - pos, buffer); 
  } else if (lua_iscfunction(state, -1 )) {
    ldb_output("Cannot break on C Function '%.*s'.\n", len - pos, buffer);
  } else if (lua_isfunction(state, -1) ) {
    char tmp[512];
    int bk;
    strcpy(tmp, buffer);

    if (p == &(input->buffer[1][0])) {
      bk = add_func_break_point(ldb, tmp, "golbal");
    } else {
      bk = add_func_break_point(ldb, tmp, "local");
    }
    if (bk >= 0) {
      ldb_output("breakpoint %d is set at '%s'\n", bk, input->buffer[1]);
      enable_func_hook(state, ldb, 1);
    }
  } else {
    ldb_output("'%.*s' is not a function!\n", len - pos, buffer);
  }

  lua_settop(state, top);
}

static int
break_handler(lua_State *state, ldb_t *ldb,
              lua_Debug *ar, input_t *input) {
  char *dv;

  if (input->num == 1) {
    ldb_output("usage:\n b(break) function\nb(break) filename:line\n");
    return 0;
  }

  dv = strchr(input->buffer[1], ':');
  if (dv) {
    handle_line_break(state, ldb, ar, input, dv);
  } else {
    handle_func_break(state, ldb, ar, input);
  }
  return 0;
}

static int
continue_handler(lua_State *state, ldb_t *ldb,
                 lua_Debug *ar, input_t *input) {
  ldb->step = 0;
  //enable_line_hook(state, 0);
  ldb->call_depth = -1;

  return -1;
}

static int
info_handler(lua_State *state, ldb_t *ldb,
             lua_Debug *ar, input_t *input) {
  int i;
  ldb_breakpoint_t *bkpoint;

  ldb_output("Num\tEnable\t\tFunc\t\tFile\t\tLine\tHit\n");
  /* search break point by file:line */
  for (i = 0; i < ldb->bknum; ++i) {
    bkpoint = &(ldb->bkpoints[i]);
    ldb_output("%d\t%d\t\t%s\t\t%s\t\t%d\t%d\n",
              i, bkpoint->active,
              bkpoint->func ? bkpoint->func : "Nil",
              bkpoint->file ? bkpoint->file : "Nil",
              bkpoint->line, bkpoint->hit);
  }

  return 0;
}

static int
enable_break_point(ldb_t *ldb, input_t *input, int enable) {
  int bk;

  bk = atoi(input->buffer[1]);
  if (bk < -1 || bk > MAX_BREAKPOINT) {
    ldb_output("breakpoint %d invalid\n", bk);
    return -1;
  }
  if (ldb->bkpoints[bk].available == 1) {
    ldb_output("breakpoint %d not set\n", bk);
    return -1;
  }
  ldb->bkpoints[bk].active = enable;
  return 0;
}

static int
disable_handler(lua_State *state, ldb_t *ldb,
                lua_Debug *ar, input_t *input) {
  enable_break_point(ldb, input, 0);
  return -1;
}

static int
enable_handler(lua_State *state, ldb_t *ldb,
               lua_Debug *ar, input_t *input) {
  enable_break_point(ldb, input, 1);
  return -1;
}

static int
delete_handler(lua_State *state, ldb_t *ldb,
               lua_Debug *ar, input_t *input) {
  int bk;

  bk = atoi(input->buffer[1]);
  if (bk < -1 || bk > MAX_BREAKPOINT) {
    ldb_output("breakpoint %d invalid\n", bk);
    return -1;
  }
  ldb->bkpoints[bk].available = 1;
  if (ldb->bkpoints[bk].file) {
    free(ldb->bkpoints[bk].file);
  }
  if (ldb->bkpoints[bk].func) {
    free(ldb->bkpoints[bk].func);
  }
  return -1;
}

static int
reload_handler(lua_State *state, ldb_t *ldb,
               lua_Debug *ar, input_t *input) {
  if (ldb->reload) {
    ldb->reload(state, input->buffer[1]);
  }

  continue_handler(state, ldb, ar, input);

  return 0;
}
