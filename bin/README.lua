-- Manual registration
local vec = require("vec")

-- Iterate over members
for field_iter in ObjectView.ReflMngr:GetFields(vec):range() do
    local field = field_iter:__deref()
    print(field.name:GetView())
end

for method_iter in ObjectView.ReflMngr:GetMethods(vec):range() do
    local method = method_iter:__deref()
    print(method.name:GetView())
end

-- Constructing types
local v = SharedObject.new(vec)
print(v:GetType():GetName()) -- prints "vec"

-- Set/get variables
v.x = 3
v.y = 4
print("x: " .. tostring(v.x))

-- Invoke Methods
print("norm2: " .. tostring(v:norm2()))

-- Iterate over variables
-- TODO
