{
  cnt[$1]+=1;
  sum[$1]+=$2;
  el[$1][cnt[$1]]=$2;
}END{
  for(i in cnt){ 
    avg[i]=sum[i]/cnt[i];
    for(j=1;j<=cnt[i];j++){
      sumvar[i] += (avg[i] - el[i][j])^2
    }
    print i,avg[i],sqrt(sumvar[i]/cnt[i])
  }
}
