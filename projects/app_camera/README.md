
# Create lv_i8n file

```shell
# install npm
sudo apt install npm

# intall lv_i18n
sudo npm i lv_i18n -g

# create en-GB.yml, and edit it
mkdir i18n
touch i18n/en-GB.yml

# create lv_i18n file
lv_i18n compile -t i18n/en-GB.yml -o i18n
```