# Copyright © 2017-2018 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


libdrm_vivante = shared_library(
  'drm_vivante',
  [
    files(
      'vivante_bo.c',
    ),
    config_file
  ],
  include_directories : [inc_root, inc_drm],
  link_with : libdrm,
  c_args : libdrm_c_args,
  dependencies : [dep_threads, dep_rt, dep_atomic_ops],
  version : '1.0.0',
  install : true,
)

pkg.generate(
  name : 'libdrm_vivante',
  libraries : libdrm_vivante,
  subdirs : ['.', 'libdrm'],
  version : meson.project_version(),
  requires_private : 'libdrm',
  description : 'Userspace interface to Vivante kernel DRM services',
)

ext_libdrm_vivante = declare_dependency(
  link_with : [libdrm, libdrm_vivante],
  include_directories : [inc_drm, include_directories('.')],
)
