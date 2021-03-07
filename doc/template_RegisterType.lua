local t = UDRefl.RegisterType({
    type = "type_name",

    -- [optional]
    attrs = {
        {
            type = "attr_type_name",
            init_args = { arg, "..." }  -- [optional]
        },
        "..."
    }
    
    -- [optional]
    bases = { "base_name",  "..." },

    -- [optional]
    fields = {
        {
            type = "field_type_name",
            name = "field_name",

            -- [optional]
            attrs = {
                {
                    type = "attr_type_name",
                    init_args = { arg, "..." }  -- [optional]
                },
                "..."
            }
        },
        "..."
    },

    -- [optional]
    unowned_fields = {
        {
            type = "field_type_name",
            name = "field_name",
            init_args = { arg, "..." }, -- [optional]

            -- [optional]
            attrs = {
                {
                    type = "attr_type_name",
                    init_args = { arg, "..." }  -- [optional]
                },
                "..."
            }
        },
        "..."
    },

    -- [optional]
    methods = {
        {
            name = "method_name",
            flag = "Variable", -- Variable / Const / Static
            result = "result_type_name", -- [optional]
            params = { "param_type_name", "..." }

            -- [optional]
            attrs = {
                {
                    type = "attr_type_name",
                    init_args = { arg, "..." }  -- [optional]
                },
                "..."
            }
        },
        "..."
    }
})
