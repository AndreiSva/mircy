project('mircy', 'c', version : '0.1', license : 'ISC',
     default_options : ['c_std=c99'])
add_global_arguments(['-Wpedantic'], language : 'c')
subdir('src')
executable('mircy', srcs, dependencies : dependency('curses'))
