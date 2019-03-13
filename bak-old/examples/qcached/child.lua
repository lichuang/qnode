local server = {}

local COMMAND_TOKEN = 1
local KEY_TOKEN     = 2
local TOKEN_FLAGS   = 3
local TOKEN_EXPIRY  = 4
local TOKEN_VLEN    = 5

local NREAD_ADD     = 1
local NREAD_SET     = 2
local NREAD_REPLACE = 3
local NREAD_APPEND  = 4
local NREAD_PREPEND = 5
local NREAD_CAS     = 6

function tokenize_command(_buffer)
  local pos = 0
  local start = 0
  local tokens = {}
  local length = 0
  local c = ""
  local left = 0

  length = qlbuffer_rlen(_buffer)

  while pos < length do
    c = qlbuffer_get(_buffer, pos, 1)

    if c == " " then
      if start ~= pos then
        qlbuffer_set(_buffer, pos, "\0")
        local str = qlbuffer_get(_buffer, start, pos - start)
        local data = {}
        data.value  = str
        data.length = pos - start
        table.insert(tokens, data)
      end

      start = pos + 1
    elseif c == "\n" then
      if qlbuffer_get(_buffer, pos - 1, 1) == "\r" then
        pos = pos - 1
        qlbuffer_set(_buffer, pos, "\0")
      end
      
      left = length - pos - 2
      break
    end

    pos = pos + 1
  end

  if start ~= pos then
    local str = qlbuffer_get(_buffer, start, pos - start)
    local data = {}
    data.value  = str
    data.length = pos - start
    table.insert(tokens, data)
  end

  return tokens, left, pos + 2
end

function process_update_command(_buffer, _tokens, _ntokens,
                                _comm, _handle_cas, _left, _pos)
  local key = ""
  local nkey = 0
  local flag = 0
  local exptime = 0
  local vlen = 0
  local data = {}
  local c = ""
  local start = 0

  key     = _tokens[KEY_TOKEN].value
  nkey    = _tokens[KEY_TOKEN].length

  flag    = qlstring_toul(_tokens[TOKEN_FLAGS].value)
  exptime = qlstring_toul(_tokens[TOKEN_EXPIRY].value)
  vlen    = qlstring_toul(_tokens[TOKEN_VLEN].value)

  data.cmd      = "set"
  data.key      = key
  data.flag     = flag
  data.exptime  = exptime
  data.vlen     = vlen

  if _left == vlen + 2 then
    length = qlbuffer_rlen(_buffer)

    start = _pos
    while _pos < length do
      c = qlbuffer_get(_buffer, _pos, 1)
      if c == "\r" then
        qlbuffer_set(_buffer, _pos, "\0")
        local str = qlbuffer_get(_buffer, start, _pos - start)
        data.value = str
        return data
      end

      _pos = _pos + 1
    end
  end
end

function process_get_command(_buffer, _tokens, _ntokens)
  for i, data in ipairs(_tokens) do
    qlog(i .. " : ".. data.value .. ", len: " .. data.length)
  end
  local i = 2
  local data = {}
  data.cmd = "get"
  while i <= _ntokens do
    data.key = _tokens[i].value
    --qnode_send(storage_id, data)
    --local arg = qnode_recv()
    qlog("data: " .. data.key)
    
    i = i + 1
  end

  return data
end

function process_command(_buffer, _tokens, _left, _pos)
  local ntokens = #_tokens

  local cmd = _tokens[COMMAND_TOKEN].value
  if cmd == "set" then
    return process_update_command(_buffer, _tokens, ntokens,
                                  NREAD_SET, false, _left, _pos)
  end

  if cmd == "get" then
    return process_get_command(_buffer, _tokens, ntokens)
  end
end

function main_loop(_args)
  local socket        = _args["sock"]
  local storage_id    = _args["storage_id"]
  -- recv data from the socket
  while true do
    local aid = qlnode_self();
    qlog("child " .. tostring(aid))
    local ret, err = qltcp_recv(socket)
    if not ret then
      qlerror(err)
      qlog("node exit");
      qlnode_exit()
      return
    end
    buffer, ret = qltcp_inbuf(socket)
    if buffer == nil then
      qlerror("reason: " .. ret)
      qlnode_exit()
      return
    end

    local tmp = qlbuffer_get(buffer, 0)
    qlog("buffer: " .. tmp)
    local tokens, left, pos = tokenize_command(buffer)
    local data = process_command(buffer, tokens, left, pos)

    qlog("out of process_command")
    for k, v in pairs(data) do
      qlog(k .. " : ".. v)
    end

    if data ~= nil then
      qlog("cmd: " .. data.cmd)
      qlnode_send(storage_id, data)
      local arg = qlnode_recv()
      for k, v in pairs(arg) do
	      qlog("response k: " .. k .. ", v: " .. v)
      end

      qlbuffer_reset(buffer);
      local out = qltcp_outbuf(socket);
      qlbuffer_write_string(out, arg.response);
      local nret, reason = qltcp_send(socket)
      if not nret then
	qlog("qtcp_send error: " .. reason)
        qlnode_exit()
      else
	qlog("qtcp_send: " .. tostring(nret))
      end
    end
  end
end

server.child = function (_args)
  local socket        = _args["sock"]
  local storage_id    = _args["storage_id"]
  -- attach the socket to the actor
  qlnode_attach(socket)

  main_loop(_args)
end

_G["child"] = server
