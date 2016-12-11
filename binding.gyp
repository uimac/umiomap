{
	"targets": [
	{
		"target_name": "umiomap",
		"conditions": [
				[	"OS == 'win'", {
					"libraries": [
					"<(module_root_dir)/lib/umio/x64/Release/umio.lib",
					"<(module_root_dir)/lib/msgpack/x64/Release/msgpack.lib",
					],
					"configurations": {
						'Debug': {
							'msvs_settings': {
									'VCCLCompilerTool': {
										'RuntimeLibrary': '3' # /MDd
								},
							},
						},
						'Release': {
							'msvs_settings': {
									'VCCLCompilerTool': {
										'RuntimeLibrary': '2' # /MD
								},
							},
						},
					},
					"msvs_configuration_attributes": {
						'CharacterSet': '1'
					},
					'defines': [ '_HAS_EXCEPTIONS=1' ],
					'msvs_settings': {
						'VCCLCompilerTool': {
						'ExceptionHandling': '1',  # /EHsc
						'AdditionalOptions': [ '/GR' ]
						},
					}
				}
			]
		],
		"sources": [
		"src/umiomap.cpp"
		],
		"include_dirs": [
		"<(module_root_dir)/lib/umio/include",
		"<(module_root_dir)/lib/msgpack/include",
		"<(module_root_dir)/src/",
		]
	}]
}
