#!/usr/bin/env python
# encoding: utf-8

__author__ = 'gclxry@gmail.com'

import argparse
import logging
import os
import sys
import multiprocessing

def get_current_directory():
    """获得脚本当前运行的目录"""
    current_directory = os.path.split(os.path.realpath(__file__))[0]
    return current_directory

def is_init_mode():
    """判断是否是初始化模式"""
    current_path = get_current_directory()
    # 根据当前目录是否存在src目录判断
    src_path = os.path.join(current_path, 'src')
    return os.path.exists(src_path)

def get_boto_config_path():
    """获得boto config 路径"""
    current_path = get_current_directory()
    if is_init_mode():
        boto_config_path = os.path.join(current_path, 'boto.cfg')
    else:
        boto_config_path = os.path.join(os.path.dirname(current_path), 'boto.cfg')
    return boto_config_path

def get_gclient_path():
    """获得gclient路径"""
    current_path = get_current_directory()
    if is_init_mode():
        gclient_path = os.path.join(current_path, '.gclient')
    else:
        gclient_path = os.path.join(os.path.dirname(current_path), '.gclient')
    return gclient_path

def init_boto_config(config):
    """初始化boto config"""
    boto = """[Boto]
proxy=%s
proxy_port=%s
"""
    boto_config_path = get_boto_config_path()
    with open(boto_config_path, 'w') as f:
        t = boto % (config['proxy_host'], config['proxy_port'])
        f.write(t)

def init_environ(config):
    """初始化环境变量"""
    env = {}
    env['http_proxy'] = '%s:%s' % (config['proxy_host'], config['proxy_port'])
    env['https_proxy'] = '%s:%s' % (config['proxy_host'], config['proxy_port'])
    env['DEPOT_TOOLS_WIN_TOOLCHAIN'] = '0'
    env['GYP_GENERATORS'] = 'msvs-ninja,ninja'
    env['GYP_MSVS_VERSION'] = config['vs_version']
    env['DEPOT_TOOLS_UPDATE'] = '0'
    env['NO_AUTH_BOTO_CONFIG'] = get_boto_config_path()

    for k, v in env.items():
        os.environ[k] = v
        logging.info('set %s=%s' % (k, os.environ[k]))

def init_gclient_config(config):
    """初始化gclient config"""
    gclient = """
solutions = [
  { "name"        : 'src',
    "url"         : 'https://chromium.googlesource.com/chromium/src.git',
    "deps_file"   : 'DEPS',
    "managed"     : False,
    "custom_deps" : {
    },
    "custom_vars": {
      "checkout_pgo_profiles": True,
    },
  },
]

"""
    gclient_path = get_gclient_path()
    with open(gclient_path, 'w') as f:
        f.write(gclient)

def sync(config):
    """同步代码"""
    # 初始化chromium.gclient文件
    init_gclient_config(config)

    if is_init_mode():
        cmd = 'gclient sync --with_branch_heads --force --gclientfile=.gclient'
    else:
        cmd = 'cd .. && gclient sync --with_branch_heads --force --gclientfile=.gclient'
    if config['is_nohooks']:
        cmd = cmd + " --nohooks"
    logging.info('cmd:%s' % cmd)
    os.system(cmd)

def runhooks(config):
    """执行glient runhooks"""
    # 初始化chromium.gclient文件
    init_gclient_config(config)
    if is_init_mode():
        cmd = 'gclient runhooks'
    else:
        cmd = 'cd .. && gclient runhooks'
    logging.info('cmd:%s' % cmd)
    os.system(cmd)

def gen(config):
    """生成工程"""
    if is_init_mode():
        logging.info('run this cmd must in src directory')
        return

    out_dir = '%s_%s' % (config['build_type'], config['arch_type'])
    if config['build_type'] == 'debug':
        cmd = r'gn gen out/%s --ide=vs%s --filters=//base:*;//content:*;//chrome:*;//chrome/browser/ui:*;//chrome/browser/ui/views:*;//ui/aura:*;//content/shell:*;//chrome/installer/mini_installer:*;//chrome/installer/setup:*;//ui/views/examples:* --sln=1chrome_debug --no-deps --args="is_debug=true is_component_build=true target_cpu =\"%s\" symbol_level=2 enable_nacl = false is_clang = true ffmpeg_branding=\"Chrome\" proprietary_codecs=true google_api_key=\"\" google_default_client_id=\"\" google_default_client_secret=\"\" "'
        cmd = cmd % (out_dir, config['vs_version'], config['arch_type'])
        logging.info('cmd:%s' % cmd)
        os.system(cmd)
        return

    if config['build_type'] == 'release':
        cmd = r'gn gen out/%s --ide=vs%s --filters=//base:*;//content:*;//chrome:*;//chrome/installer/mini_installer:*;//chrome/installer/setup:* --sln=1chrome_release --no-deps --args="is_official_build=true use_lld=true is_debug=false is_component_build=false target_cpu =\"%s\" enable_nacl = false is_clang = true ffmpeg_branding=\"Chrome\" proprietary_codecs=true google_api_key=\"\" google_default_client_id=\"\" google_default_client_secret=\"\" "'
        cmd = cmd % (out_dir, config['vs_version'], config['arch_type'])
        logging.info('cmd:%s' % cmd)
        os.system(cmd)
        return

def build(config):
    """执行编译工程"""
    if is_init_mode():
        logging.info('run this cmd must in src directory')
        return
    out_dir = '%s_%s' % (config['build_type'], config['arch_type'])
    cpu_count = multiprocessing.cpu_count()
    job = cpu_count + 2
    if config['build_type'] == 'debug':
        cmd = 'ninja -C out/%s chrome -j %d' % (out_dir, job)
    if config['build_type'] == 'release':
        cmd = 'ninja -C out/%s mini_installer -j %d' % (out_dir, job)
    logging.info('cmd:%s' % cmd)
    os.system(cmd)
    return

def main():
    logging_format = '[%(levelname)s %(filename)s:%(lineno)d] %(message)s'
    logging.root.handlers = []
    logging.basicConfig(format=logging_format, level=logging.INFO)

    config = {}
    config['is_nohooks'] = False
    config['vs_version'] = '2019'
    config['proxy_host'] = '127.0.0.1'
    config['proxy_port'] = '8807'
    config['build_type'] = 'debug'
    config['arch_type'] = 'x86'

    build_type = ['debug', 'release']
    arch_type = ['x64', 'x86']

    parser = argparse.ArgumentParser(description='Chromium tool')
    parser.add_argument('-s', '--sync', action='store_true', help='call gclient sync')
    parser.add_argument('-r', '--runhooks', action='store_true', help='call gclient runhooks')
    parser.add_argument('-g', '--gen', choices=build_type, help='gen project')
    parser.add_argument('-b', '--build', choices=build_type, help='build project')
    parser.add_argument('-a', '--arch', choices=arch_type, help='arch type')

    parser.add_argument('-n', '--nohooks', action='store_true', help='not run hooks, defualt is runhooks')
    parser.add_argument('-v', '--vs-version', help='the vs version, default is 2019')

    args = parser.parse_args()
    if not (args.runhooks or args.sync or args.gen or args.build):
        parser.print_help()
        return

    if args.nohooks:
        config['is_nohooks'] = True
    if args.vs_version:
        config['vs_version'] = args.vs_version
    if args.arch:
        config['arch_type'] = args.arch

    init_environ(config)
    init_boto_config(config)

    if args.sync:
        sync(config)
        return

    if args.runhooks:
        runhooks(config)
        return

    if args.gen:
        config['build_type'] = args.gen
        gen(config)
        return

    if args.build:
        config['build_type'] = args.build
        build(config)
        return

if __name__ == '__main__':
    sys.exit(main())