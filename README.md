# Lea firmware

## Install SDKs

1. Download and install sdks
```bash
$ mkdir ~/esp

# install esp-idf
$ cd ~/esp
$ git clone --recursive https://github.com/espressif/esp-idf.git
$ cd esp-idf
$ ./install.sh

# install esp-adf
$ cd ..
$ git clone --recursive https://github.com/espressif/esp-adf.git

```

2. Add those lines to your **~/.zshrc**
```bash
export IDF_PATH="${HOME}/esp/esp-idf"
export ADF_PATH="${HOME}/esp/esp-adf"
alias get_idf='. ${HOME}/esp/esp-idf/export.sh'
```
