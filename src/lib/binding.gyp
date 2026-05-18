{
    "targets": [
        {
            "target_name": "sha_core",
            "type": "static_library",
            "sources": [
                "../sha/md5.cpp",
                "../sha/sha1.cpp"
            ],
            "cflags_cc": [ "-fexceptions" ],
            "include_dirs": [
                "../sha/include"
            ],
            "direct_dependent_settings": {
                "include_dirs": [
                    "../sha/include"
                ]
            }
        },
        {
            "target_name": "md5_v2",
            "cflags_cc": [ "-fexceptions" ],
            "sources": ["node_md5.cpp"],
            "include_dirs": [
                "include"
            ],
            "dependencies": [
                "sha_core"
            ]
        }
    ]
}