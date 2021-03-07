local t = UDRefl.RegisterType({
    -----------
    -- basic --
    -----------

    type = "type_name",
    
    -- [optional]
    bases = { "base_name",  "..." },

    -- [optional]
    fields = {
        { type = "field_type_name", name = "field_name" },
        { "..." }
    },

    -- [optional]
    methods = {
        {
            name = "method_name",
            flag = "Variable", -- Variable / Const / Static
            result = "result_type_name", -- [optional]
            params = { "param_type_name", "..." }
        },
        { "..." }
    },

    ---------
    -- pro --
    ---------

    -- [optional]
    unowned_fields = {
        {
            type = "field_type_name",
            name = "field_name"
            init_args = { arg, "..." } -- [optional]
        },
        { "..." }
    },
})
