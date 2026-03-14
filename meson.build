project('ASCIIBlocks', 'c', version : '1.5.0')
ncurses_dep = dependency('ncurses', required: true)
executable('asciiblocks',
  sources: ['asciiblocks.c'],
  dependencies: [ncurses_dep],
  c_args : ['-DMESON_VERSION="' + meson.project_version() + '"']
)
