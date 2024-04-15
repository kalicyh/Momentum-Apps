# Flipper zero wiegand plugin

Add as git submodule


Add to your `applicaiton.fam`
```
App(
    appid="plugin_wiegand",
    apptype=FlipperAppType.PLUGIN,
    entry_point="plugin_wiegand_ep",
    requires=["seader"],
    sources=["plugin/wiegand.c"],
)
```
