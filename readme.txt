cgi.cfg配置如下：
locale_lang_path=/usr/local/nagios/share/locale
locale_lang_user=nagiosadmin|zh_CN,nagiosadmin2|en

说明：locale_lang_path是语言包的位置
      locale_lang_user是指不同的登录用户显示的语言，如上面，nagiosadmin显示中文，nagiosadmin2显示英文。


nagios是源码包，基于nagios-3.5.1.tar修改

locale是现在翻译的中文语言包

在ubuntu中出现语言包不起作用情况：

需要在/var/lib/locales/supported.d/local中添加相应的文件包
如：zh_CN.UTF-8 UTF-8
在Ubuntu下，修改 /var/lib/locales/supported.d/local 文件，配置新的 locale，然后运行 locale-gen 命令。