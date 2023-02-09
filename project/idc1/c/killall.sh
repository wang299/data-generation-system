####################################################################
# 停止数据中心后台服务程序的脚本。
####################################################################

killall -9 procctl
killall gzipfiles crtsurfdata deletefiles ftpgetfiles ftpputfiles tcpputfiles tcpgetfiles fileserver obtcodetodb execsql obtmindtodb dminingmysql xmltodb syncupdate syncincrement syncupdate syncincrement 
killall deletetable migratetable  deletetable_oracle xmltodb_oracle migratetable_oracle syncupdate_oracle syncincrement_oracle

sleep 3

killall -9 gzipfiles crtsurfdata deletefiles ftpgetfiles ftpputfiles tcpputfiles tcpgetfiles fileserver obtcodetodb execsql obtmindtodb dminingmysql xmltodb syncupdate syncincrement syncupdate syncincrement
killall -9 deletetable migratetable  deletetable_oracle xmltodb_oracle migratetable_oracle syncupdate_oracle syncincrement_oracle

