load("//setcoveringsolver:variables.bzl", "STDCPP",
         "COINOR_COPTS", "COINOR_DEP",
         "CPLEX_COPTS", "CPLEX_DEP",
         "GUROBI_COPTS", "GUROBI_DEP")

cc_library(
        name = "greedy",
        hdrs = ["greedy.hpp"],
        srcs = ["greedy.cpp"],
        deps = ["//setcoveringsolver:setcoveringsolver"],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "milp_cbc",
        hdrs = ["milp_cbc.hpp"],
        srcs = ["milp_cbc.cpp"],
        deps = [
                "//setcoveringsolver:setcoveringsolver",
        ] + COINOR_DEP,
        copts = COINOR_COPTS,
        visibility = ["//visibility:public"],
)

cc_library(
        name = "milp_gurobi",
        hdrs = ["milp_gurobi.hpp"],
        srcs = ["milp_gurobi.cpp"],
        deps = [
                "//setcoveringsolver:setcoveringsolver",
        ] + GUROBI_DEP,
        copts = GUROBI_COPTS,
        visibility = ["//visibility:public"],
)

cc_library(
        name = "localsearch_rowweighting",
        hdrs = ["localsearch_rowweighting.hpp"],
        srcs = ["localsearch_rowweighting.cpp"],
        deps = [
                ":greedy",
                "//setcoveringsolver:setcoveringsolver",
                "@optimizationtools//optimizationtools/utils:utils",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "largeneighborhoodsearch",
        hdrs = ["largeneighborhoodsearch.hpp"],
        srcs = ["largeneighborhoodsearch.cpp"],
        deps = [
                ":greedy",
                "//setcoveringsolver:setcoveringsolver",
        ],
        visibility = ["//visibility:public"],
)

cc_library(
        name = "algorithms",
        hdrs = ["algorithms.hpp"],
        srcs = ["algorithms.cpp"],
        deps = [
                ":greedy",
                ":milp_cbc",
                ":milp_gurobi",
                ":localsearch_rowweighting",
                ":largeneighborhoodsearch",
                "@boost//:program_options",
        ] + COINOR_DEP + GUROBI_DEP,
        copts = COINOR_COPTS + GUROBI_COPTS,
        visibility = ["//visibility:public"],
)

