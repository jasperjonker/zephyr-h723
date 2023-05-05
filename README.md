# Zephyr on NUCLEO-H723ZG
## Installation
1) Install Zephyr (SDK, depencendies, etc.), follow the instructions [here](https://docs.zephyrproject.org/latest/getting_started/index.html)
    - Currently using the `Zephyr SDK 0.16.0`
2) We use the [zephyr workspace application structure](https://docs.zephyrproject.org/latest/develop/application/index.html#zephyr-workspace-application). Clone this repository. It should look like:
    ```
    zephyrproject/
    ├─── .vscode/
    │    └─── launch.json
    │    └─── ...
    ├─── .west/
    │    └─── config
    ├─── zephyr/
    ├─── bootloader/
    ├─── modules/
    ├─── tools/
    └─── applications/
        └── boards/
        └── ...
    ```

    This git repo manages mainly the `applications` folder. The `zephyr` is version is set manually (e.g. `git checkout v3.0.0`), and `west update` is used to update the other folders.
