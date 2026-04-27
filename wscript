#
# This file is the default set of rules to compile a Pebble project.
#
# Feel free to customize this to your needs.
#

top = '.'
out = 'build'

def options(ctx):
    ctx.load('pebble_sdk')

def configure(ctx):
    ctx.load('pebble_sdk')

def build(ctx):
    ctx.load('pebble_sdk')

    build_worker = os.path.exists('worker_src')
    binaries = []

    cached_env = ctx.env
    for p in ctx.env.TARGET_PLATFORMS:
        ctx.set_env(ctx.all_envs[p])
        ctx.set_group(ctx.env.PLATFORM_NAME)
        app_elf='{}/pebble-app.elf'.format(ctx.env.BUILD_DIR)
        ctx.pebble_build_group(
            includes=['src'],
            sources=ctx.path.ant_glob('src/**/*.c'),
            target=app_elf,
        )
        binaries.append({'platform': p, 'app_elf': app_elf})
        if build_worker:
            worker_elf='{}/pebble-worker.elf'.format(ctx.env.BUILD_DIR)
            ctx.pebble_build_group(
                sources=ctx.path.ant_glob('worker_src/**/*.c'),
                target=worker_elf,
                worker=True,
            )
            binaries.append({'platform': p, 'worker_elf': worker_elf})
    ctx.set_env(cached_env)
    ctx.set_group('bundle')
    ctx.pebble_bundle(binaries=binaries, js=ctx.path.ant_glob('src/pkjs/**/*.js'))
