
function qlog(...)
  qllog(string.format(...))
end

function qerror(...)
  qlerror(string.format(...))
end

function require_ex(name)
  qlog( string.format("require_ex = %s", name))
  if package.loaded[name] then
    qlog(string.format("require_ex module[%s] reload", mname))
  end 
  package.loaded[name] = nil 
  require(name)
end
