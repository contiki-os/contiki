{cnt[$1]+=1;sum[$1]+=$2}END{for(i in cnt) print i,sum[i]/cnt[i]}
