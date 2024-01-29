workspace("fucksepples-webshit")
    configurations({"Debug", "Release"})

project("fucksepples-webshit")
    kind("ConsoleApp")
    language("C++")
    cppdialect("C++11")
    targetdir("bin/%{cfg.buildcfg}")

    --buildoptions({""})

    --libdirs({"/usr/local/lib"})
    links({"dl", "pthread"})

    files({
        "**.cpp",
        "**.c",
        "**.h"
    })

    removefiles({
        "**/tests/**",
        "**/examples/**",
        "**/tools/**",
        "**/samples/**",
        "**/docs/**",
    })


    filter "configurations:Debug"
        defines({"DEBUG"})
        symbols("On")

    filter "configurations:Release"
        defines({"NDEBUG"})
        optimize("On")
