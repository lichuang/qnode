
function qlog(...)
  qllog(string.format(...))
end

function qerror(...)
  qlerror(string.format(...))
end

function require_ex( _mname )
  qlog( string.format("require_ex = %s", _mname) )
  if package.loaded[_mname] then
    qlog( string.format("require_ex module[%s] reload", _mname))
  end 
  package.loaded[_mname] = nil 
  require( _mname )
end
