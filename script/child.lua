local child = {}

child.test = function (_args)
  print("in child, id:" .. _args["id"])
end

_G["child"] = child
