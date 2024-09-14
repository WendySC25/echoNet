project('echoNet', 'c',
  version: '1.0',
  default_options: ['warning_level=3', 'c_std=c11'])

# Define los directorios
src_dir = 'src'
cjson_dir = 'third_party/cJSON'

# Define las fuentes comunes
common_sources = [
  src_dir / 'message.c',
  cjson_dir / 'cJSON.c',
  src_dir / 'protocol.c'
]

# Define las fuentes específicas para cada ejecutable
servidor_sources = common_sources + [
  src_dir / 'chatServer.c',
  src_dir / 'server.c',
  src_dir / 'user.c'
]

cliente_sources = common_sources + [
  src_dir / 'chatClient.c',
  src_dir / 'client.c',
  src_dir / 'server.c',
  src_dir / 'user.c',
  src_dir / 'clientInterface.c'
]

glib_dep = dependency('glib-2.0', required: true)

executable('servidorR',
  sources: servidor_sources,
  dependencies: glib_dep)

executable('clienteR',
  sources: cliente_sources,
  dependencies: glib_dep)

