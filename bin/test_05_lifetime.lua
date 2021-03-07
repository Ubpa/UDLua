-- Manual registration
local C = UDRefl.RegisterType({
    type = "C",
    methods = {
        {
            name = "__ctor",
            body = function (obj)
                print("ctor " .. obj:GetType():GetName())
            end
        },
        {
            name = "__dtor",
            body = function (obj)
                print("dtor " .. obj:GetType():GetName())
            end
        }
    }
})

local c0 = SharedObject.new(C)
local c1 = SharedObject.new(C)

c0:Reset() -- call c0:__dtor

-- clear
print("run : c0 = nil")
c0 = nil
print("run : c1 = nil")
c1 = nil
print("run : collectgarbage()")
collectgarbage() -- run c1:__dtor here
ObjectView.ReflMngr.typeinfos:erase(C)
