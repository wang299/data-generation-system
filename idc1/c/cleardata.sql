delete from t_zhobtind where ddatetime<timestampadd(minute,-120,now());
delete from t_zhobtind1 where ddatetime<timestampadd(minute,-120,now());
delete from t_zhobtind2 where ddatetime<timestampadd(minute,-120,now());
delete from t_zhobtind3 where ddatetime<timestampadd(minute,-120,now());
