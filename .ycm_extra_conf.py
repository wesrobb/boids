def Settings( **kwargs ):
  return {
    'flags': [ '-x', 'c', '-Wall', '-Wextra', '-Werror', '-I', 'lib/SDL2-2.0.10/include', '-I', 'lib/cglm/include' ],
  }
