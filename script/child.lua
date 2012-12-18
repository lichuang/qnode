local server = {}
--function test(_tbl)
server.test = function ()
  --print("id: %d", _tbl[id])
  print("id: %d", 1000)
end

_G["child"] = server
