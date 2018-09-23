if(invLastPeriodTR>invTrMetric){
    DLOG(INFO)<<"begin release lock strategy.invLastPeriodTR="+boost::lexical_cast<string>(invLastPeriodTR)+">invTrMetric="+boost::lexical_cast<string>(invTrMetric);
    BreakoutInfo tmpBreakoutInfo;//返回当前tick的网格状态，突破状态
    isBreakout(lastPrice,tmpBreakoutInfo,allNetInfo_lock,lastNetInfo_lock);
    DLOG(INFO)<<"lastNetInfo_lock:lastPrice="+boost::lexical_cast<string>(lastNetInfo_lock->lastPrice)+
                ";netID="+boost::lexical_cast<string>(lastNetInfo_lock->netID)+
                ";price1="+boost::lexical_cast<string>(lastNetInfo_lock->price1)+
                ";price2="+boost::lexical_cast<string>(lastNetInfo_lock->price2);
    DLOG(INFO)<<"tmpBreakoutInfo:beforeNetID="+boost::lexical_cast<string>(tmpBreakoutInfo.beforeNetID)+
                ";afterNetID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+
                ";available="+boost::lexical_cast<string>(tmpBreakoutInfo.available)+
                ";breakoutStatus="+boost::lexical_cast<string>(tmpBreakoutInfo.breakoutStatus);
    if(!tmpBreakoutInfo.available){
        LOG(ERROR)<<"ERROR:some thing is wrong in isBreakout";
        return;
    }
    if(invProcessState=="0"){
        DLOG(INFO)<< "波动="+boost::lexical_cast<string>(invLastPeriodTR)+"，超多阈值="+boost::lexical_cast<string>(invTrMetric)+
                     "，进入第1阶段，开始检测价格是否突破。";
        invProcessState="1";//设置阶段
        lastNetInfo_lock->lastPrice=0;
        lastNetInfo_lock->netID=0;
    }else if(invProcessState=="1"){//价格突破检查阶段
        if(tmpBreakoutInfo.breakoutStatus!="-1"){//初始突破，不管突破方向。
          DLOG(INFO)<< "主动第1阶段:检测到网格被初始突破，进入第2阶段，开始检测是否真实突破。";
          invProcessState="2";
          double tmpNetPrice=0;
          if(tmpBreakoutInfo.direction=="0"){
              tmpNetPrice=getNetPrice(netIDToRange_lock,tmpBreakoutInfo.afterNetID,"d");
          }else if(tmpBreakoutInfo.direction=="1"){
              tmpNetPrice=getNetPrice(netIDToRange_lock,tmpBreakoutInfo.afterNetID,"u");
          }
          if(tmpNetPrice==0){
              LOG(ERROR)<<"ERROR:can't find net price.";
              return;
          }else{
              DLOG(INFO)<<"netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+
                          "netPrice="+boost::lexical_cast<string>(tmpNetPrice);
          }
          invBreakoutInfo->netPrice=tmpNetPrice;
          invBreakoutInfo->upPrice=tmpNetPrice+2*tickPrice;
          invBreakoutInfo->downPrice=tmpNetPrice-2*tickPrice;
          invBreakoutInfo->stopProfitNetID=0;
          DLOG(INFO)<< "突破网格线="+boost::lexical_cast<string>(invBreakoutInfo->netPrice)+",真实区间["+
                       boost::lexical_cast<string>(invBreakoutInfo->downPrice)+","+
                       boost::lexical_cast<string>(invBreakoutInfo->upPrice)+"]";
          DLOG(INFO)<<"初始突破，需要记录初始突破信息；后续的突破处理都需要根据初始信息进行.direction="+tmpBreakoutInfo.direction;
          invBreakoutInfo->breakoutStatus="1";
          invBreakoutInfo->beforeNetID=tmpBreakoutInfo.beforeNetID;
          invBreakoutInfo->afterNetID=tmpBreakoutInfo.afterNetID;
          invBreakoutInfo->direction=tmpBreakoutInfo.direction;
          invBreakoutInfo->initDirection=tmpBreakoutInfo.direction;
          DLOG(INFO)<<"初始突破，需要记录初始突破信息；后续的突破处理都需要根据初始信息进行.direction="+tmpBreakoutInfo.direction
                      +";beforeNetID="+boost::lexical_cast<string>(invBreakoutInfo->beforeNetID)
                      +";afterNetID="+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);
        }else{
            DLOG(INFO)<< "主动第1阶段:检测网格是否被初始突破.";
        }
    }else if(invProcessState=="2"){//是否真实突破阶段
        DLOG(INFO)<< "主动第2阶段:检测网格是否被真实突破,价格区间为["+boost::lexical_cast<string>(invBreakoutInfo->downPrice)+","+
                     boost::lexical_cast<string>(invBreakoutInfo->upPrice)+"]";
        if(tmpBreakoutInfo.breakoutStatus!="-1"){//检测网格是否被真实突破过程中，又有价格突破
            DLOG(INFO)<< "在检测网格是否被真实突破过程中，又有价格突破.判断如何处理。curr direction="+tmpBreakoutInfo.direction+",last direction="+invBreakoutInfo->direction;
            if(invBreakoutInfo->direction==tmpBreakoutInfo.direction){//本次突破和之前的方向一致，那么判断是否是连续突破止盈。有可能是折返之后又回到突破网格.
                if(invBreakoutInfo->direction=="0"){//向上突破，确认当前的afterNetID是否大于初始突破时候的afterNetID
                    if(tmpBreakoutInfo.afterNetID>invBreakoutInfo->afterNetID){
                        DLOG(INFO)<< "在检测网格是否被真实突破期间，价格连续上涨突破。当前netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+" 大于 初始netID="+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);
                        invBreakoutInfo->beforeNetID=tmpBreakoutInfo.beforeNetID;
                        invBreakoutInfo->afterNetID=tmpBreakoutInfo.afterNetID;
                        invBreakoutInfo->direction=tmpBreakoutInfo.direction;
                        DLOG(INFO)<< "netID change to "+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);
                    }else{
                        DLOG(INFO)<< "在上涨突破期间，价格有突破,但是突破无效.当前netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+",初始beforeNetID="+boost::lexical_cast<string>(invBreakoutInfo->beforeNetID)+",初始afterNetID="+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);
                    }
                }else if(invBreakoutInfo->direction=="1"){//向下突破，确认当前的afterNetID是否小于初始突破时候的afterNetID
                    if(tmpBreakoutInfo.afterNetID<invBreakoutInfo->afterNetID){
                        DLOG(INFO)<< "在检测网格是否被真实突破期间，价格连续下跌突破。当前netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+" 小于 初始netID="+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);
                        invBreakoutInfo->beforeNetID=tmpBreakoutInfo.beforeNetID;
                        invBreakoutInfo->afterNetID=tmpBreakoutInfo.afterNetID;
                        invBreakoutInfo->direction=tmpBreakoutInfo.direction;
                        DLOG(INFO)<< "netID change to "+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);
                    }else{
                        DLOG(INFO)<< "在下跌突破期间，价格有突破,但是突破无效。当前netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+",初始beforeNetID="+boost::lexical_cast<string>(invBreakoutInfo->beforeNetID)+",初始afterNetID="+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);
                    }
                }
            }else{//本次突破和之前的方向不一致
                if(invBreakoutInfo->direction=="1"){//init向下突破，当前向上突破。确认当前的afterNetID是否大于初始突破时候的beforeNetID
                    if(tmpBreakoutInfo.afterNetID>invBreakoutInfo->beforeNetID){
                        DLOG(INFO)<<"在检测网格是否被真实突破期间，价格折返上涨突破。当前netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+" 大于 初始beforeNetID="+boost::lexical_cast<string>(invBreakoutInfo->beforeNetID)+",当前突破方向"+tmpBreakoutInfo.direction+"，初始突破方向"+invBreakoutInfo->direction;
                        invBreakoutInfo->beforeNetID=tmpBreakoutInfo.beforeNetID;
                        invBreakoutInfo->afterNetID=tmpBreakoutInfo.afterNetID;
                        invBreakoutInfo->direction=tmpBreakoutInfo.direction;
                        DLOG(INFO)<< "netID change to "+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);
                    }else{
                        DLOG(INFO)<<"在下跌突破期间，价格有突破,但是突破无效。当前netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+",初始beforeNetID="+boost::lexical_cast<string>(invBreakoutInfo->beforeNetID)+",初始afterNetID="+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);+",当前突破方向"+tmpBreakoutInfo.direction+"，初始突破方向"+invBreakoutInfo->direction;
                    }
                }else{//初始向上突破，当前向下突破。确认当前的afterNetID是否小于初始突破时候的beforeNetID
                    if(tmpBreakoutInfo.afterNetID<invBreakoutInfo->beforeNetID){
                        DLOG(INFO)<<"在检测网格是否被真实突破期间，价格折返下跌突破。当前netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+" 小于 初始beforeNetID="+boost::lexical_cast<string>(invBreakoutInfo->beforeNetID)+",当前突破方向"+tmpBreakoutInfo.direction+"，初始突破方向"+invBreakoutInfo->direction;
                        invBreakoutInfo->beforeNetID=tmpBreakoutInfo.beforeNetID;
                        invBreakoutInfo->afterNetID=tmpBreakoutInfo.afterNetID;
                        invBreakoutInfo->direction=tmpBreakoutInfo.direction;
                        DLOG(INFO)<< "netID change to "+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);
                    }else{
                        DLOG(INFO)<<"在上涨突破期间，价格有突破,但是突破无效。当前netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+",初始beforeNetID="+boost::lexical_cast<string>(invBreakoutInfo->beforeNetID)+",初始afterNetID="+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);+",当前突破方向"+tmpBreakoutInfo.direction+"，初始突破方向"+invBreakoutInfo->direction;
                    }
                }
            }
        }else{//没有突破
        }
        //检测真实突破
        if(invBreakoutInfo->initDirection=="0"){//上涨突破
            if(lastPrice>=invBreakoutInfo->upPrice){//真实突破
                DLOG(INFO)<<"上涨真实突破;lastPrice="+boost::lexical_cast<string>(lastPrice)+" >= 真实突破线="+
                            boost::lexical_cast<string>(invBreakoutInfo->upPrice)+",设置主动做市标志:stopProfitNetID="+
                            boost::lexical_cast<string>(invBreakoutInfo->afterNetID);
                invBreakoutInfo->stopProfitNetID=invBreakoutInfo->afterNetID;
                invBreakoutInfo->upPrice=0;
                invProcessState="3";
            }else if(lastPrice<=invBreakoutInfo->downPrice){
                DLOG(INFO)<<"上涨突破检测时，价格跌破下限，突破失效，进入突破检测阶段。lastPrice="+boost::lexical_cast<string>(lastPrice)
                            +" <= 真实突破线="+boost::lexical_cast<string>(invBreakoutInfo->downPrice);
                invBreakoutInfo->stopProfitNetID=0;
                invBreakoutInfo->netPrice=0;
                invBreakoutInfo->upPrice=0;
                invBreakoutInfo->downPrice=0;
                invProcessState="1";//突破检测阶段
            }
        }else if(invBreakoutInfo->initDirection=="1"){
            if(lastPrice<=invBreakoutInfo->downPrice){//真实突破
                DLOG(INFO)<<"下跌真实突破;lastPrice="+boost::lexical_cast<string>(lastPrice)+" <= 真实突破线="+
                            boost::lexical_cast<string>(invBreakoutInfo->downPrice)+",设置主动做市标志:stopProfitNetID="+
                            boost::lexical_cast<string>(invBreakoutInfo->afterNetID);
                invBreakoutInfo->stopProfitNetID=invBreakoutInfo->afterNetID;
                invBreakoutInfo->downPrice=0;
                invProcessState="3";
            }else if(lastPrice>=invBreakoutInfo->upPrice){
                DLOG(INFO)<<"下跌突破检测时，价格跌破上限，突破失效，进入突破检测阶段。lastPrice="+
                            boost::lexical_cast<string>(lastPrice)+" >= 真实突破线="+
                            boost::lexical_cast<string>(invBreakoutInfo->upPrice);
                invBreakoutInfo->stopProfitNetID=0;
                invBreakoutInfo->netPrice=0;
                invBreakoutInfo->upPrice=0;
                invBreakoutInfo->downPrice=0;
                invProcessState="1";//突破检测阶段
            }
        }
    }else if(invProcessState=="3"){//
        DLOG(INFO)<< "主动第3阶段:主动做市阶段一,下一阶段检测阈值网格="+boost::lexical_cast<string>(invBreakoutInfo->stopProfitNetID);
        if(tmpBreakoutInfo.breakoutStatus!="-1"){//检测网格是否被真实突破过程中，又有价格突破
            DLOG(INFO)<< "在检测网格是否被真实突破过程中，又有价格突破.判断如何处理。curr direction="+tmpBreakoutInfo.direction+",last direction="+invBreakoutInfo->direction;
            if(invBreakoutInfo->direction==tmpBreakoutInfo.direction){//本次突破和之前的方向一致，那么判断是否是连续突破止盈。有可能是折返之后又回到突破网格.
                if(invBreakoutInfo->direction=="0"){//向上突破，确认当前的afterNetID是否大于初始突破时候的afterNetID
                  if(tmpBreakoutInfo.afterNetID>invBreakoutInfo->afterNetID){
                      DLOG(INFO)<< "在检测网格是否被真实突破期间，价格连续上涨突破。当前netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+" 大于 初始netID="+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);
                      invBreakoutInfo->beforeNetID=tmpBreakoutInfo.beforeNetID;
                      invBreakoutInfo->afterNetID=tmpBreakoutInfo.afterNetID;
                      invBreakoutInfo->direction=tmpBreakoutInfo.direction;
                      DLOG(INFO)<< "netID change to "+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);
                  }else{
                      DLOG(INFO)<< "在上涨突破期间，价格有突破,但是突破无效.当前netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+",初始beforeNetID="+boost::lexical_cast<string>(invBreakoutInfo->beforeNetID)+",初始afterNetID="+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);
                  }
              }else if(invBreakoutInfo->direction=="1"){//向下突破，确认当前的afterNetID是否小于初始突破时候的afterNetID
                  if(tmpBreakoutInfo.afterNetID<invBreakoutInfo->afterNetID){
                      DLOG(INFO)<< "在检测网格是否被真实突破期间，价格连续下跌突破。当前netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+" 小于 初始netID="+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);
                      invBreakoutInfo->beforeNetID=tmpBreakoutInfo.beforeNetID;
                      invBreakoutInfo->afterNetID=tmpBreakoutInfo.afterNetID;
                      invBreakoutInfo->direction=tmpBreakoutInfo.direction;
                      DLOG(INFO)<< "netID change to "+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);
                  }else{
                      DLOG(INFO)<< "在下跌突破期间，价格有突破,但是突破无效。当前netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+",初始beforeNetID="+boost::lexical_cast<string>(invBreakoutInfo->beforeNetID)+",初始afterNetID="+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);
                  }
              }
            }else{//本次突破和之前的方向不一致
              if(invBreakoutInfo->direction=="1"){//init向下突破，当前向上突破。确认当前的afterNetID是否大于初始突破时候的beforeNetID
                  if(tmpBreakoutInfo.afterNetID>invBreakoutInfo->beforeNetID){
                      DLOG(INFO)<<"在检测网格是否被真实突破期间，价格折返上涨突破。当前netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+" 大于 初始beforeNetID="+boost::lexical_cast<string>(invBreakoutInfo->beforeNetID)+",当前突破方向"+tmpBreakoutInfo.direction+"，初始突破方向"+invBreakoutInfo->direction;
                      invBreakoutInfo->beforeNetID=tmpBreakoutInfo.beforeNetID;
                      invBreakoutInfo->afterNetID=tmpBreakoutInfo.afterNetID;
                      invBreakoutInfo->direction=tmpBreakoutInfo.direction;
                      DLOG(INFO)<< "netID change to "+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);
                  }else{
                      DLOG(INFO)<<"在下跌突破期间，价格有突破,但是突破无效。当前netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+",初始beforeNetID="+boost::lexical_cast<string>(invBreakoutInfo->beforeNetID)+",初始afterNetID="+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);+",当前突破方向"+tmpBreakoutInfo.direction+"，初始突破方向"+invBreakoutInfo->direction;
                  }
              }else{//初始向上突破，当前向下突破。确认当前的afterNetID是否小于初始突破时候的beforeNetID
                  if(tmpBreakoutInfo.afterNetID<invBreakoutInfo->beforeNetID){
                      DLOG(INFO)<<"在检测网格是否被真实突破期间，价格折返下跌突破。当前netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+" 小于 初始beforeNetID="+boost::lexical_cast<string>(invBreakoutInfo->beforeNetID)+",当前突破方向"+tmpBreakoutInfo.direction+"，初始突破方向"+invBreakoutInfo->direction;
                      invBreakoutInfo->beforeNetID=tmpBreakoutInfo.beforeNetID;
                      invBreakoutInfo->afterNetID=tmpBreakoutInfo.afterNetID;
                      invBreakoutInfo->direction=tmpBreakoutInfo.direction;
                      DLOG(INFO)<< "netID change to "+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);
                  }else{
                      DLOG(INFO)<<"在上涨突破期间，价格有突破,但是突破无效。当前netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+",初始beforeNetID="+boost::lexical_cast<string>(invBreakoutInfo->beforeNetID)+",初始afterNetID="+boost::lexical_cast<string>(invBreakoutInfo->afterNetID);+",当前突破方向"+tmpBreakoutInfo.direction+"，初始突破方向"+invBreakoutInfo->direction;
                  }
              }
          }
        }else{//没有突破
        }
        if(invBreakoutInfo->initDirection=="0"){
              if(lastPrice<invBreakoutInfo->downPrice){//停止做市
                  invProcessState="1";
                  DLOG(INFO)<<"在上涨做市期间，价格回落到突破线下的真假线，突破失效。。lastPrice="+
                              boost::lexical_cast<string>(lastPrice)+",失效线="+
                              boost::lexical_cast<string>(invBreakoutInfo->downPrice);
                  return ;
              }else if(tmpBreakoutInfo.afterNetID>=invBreakoutInfo->stopProfitNetID){
                  double tmpNetPrice=getNetPrice(netIDToRange_lock,tmpBreakoutInfo.afterNetID,"d");
                  if(tmpNetPrice==0){
                      LOG(ERROR)<<"ERROR:can't find net price.";
                      return;
                  }else{
                      DLOG(INFO)<<"netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+
                                  "netPrice="+boost::lexical_cast<string>(tmpNetPrice);
                  }
                  DLOG(INFO)<<"在上涨做市期间，价格继续突破下下根网格线,进入判定阶段。currNetID="+
                              boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+",判定网格线="+
                              boost::lexical_cast<string>(invBreakoutInfo->stopProfitNetID);
                  invProcessState="4";
                  agglgsTickNums=0;
                  invBreakoutInfo->upPrice=tmpNetPrice+2*tickPrice;
                  invBreakoutInfo->downPrice=tmpNetPrice-2*tickPrice;
              }
        }else if(invBreakoutInfo->initDirection=="1"){
              if(lastPrice>invBreakoutInfo->upPrice){//停止做市
                  invProcessState="1";
                  DLOG(INFO)<<"在下跌做市期间，价格上涨到突破线上的真假线，突破失效。lastPrice="+
                              boost::lexical_cast<string>(lastPrice)+",失效线="+
                              boost::lexical_cast<string>(invBreakoutInfo->upPrice);
                  return;
              }else if(tmpBreakoutInfo.afterNetID<=invBreakoutInfo->stopProfitNetID){
                  double tmpNetPrice=getNetPrice(netIDToRange_lock,tmpBreakoutInfo.afterNetID,"u");
                  if(tmpNetPrice==0){
                      LOG(ERROR)<<"ERROR:can't find net price.";
                      return;
                  }else{
                      DLOG(INFO)<<"netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+
                                  "netPrice="+boost::lexical_cast<string>(tmpNetPrice);
                  }
                  DLOG(INFO)<<"在下跌做市期间，价格继续突破下下根网格线,进入判定阶段。currNetID="+
                              boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+",判定网格线="+
                              boost::lexical_cast<string>(invBreakoutInfo->stopProfitNetID);
                  invProcessState="4";
                  agglgsTickNums=0;
                  invBreakoutInfo->upPrice=tmpNetPrice+2*tickPrice;
                  invBreakoutInfo->downPrice=tmpNetPrice-2*tickPrice;
              }
        }
        aggOrderInsertStrategy(invBreakoutInfo->initDirection,marketdatainfo);
    }else if(invProcessState=="4"){//阶段判定，30个tick
        agglgsTickNums+=1;
        if(invBreakoutInfo->initDirection=="0"){//上涨
            if(lastPrice>=invBreakoutInfo->upPrice){
                DLOG(INFO)<<"判定阶段，价格触及突破线，继续主动做市。";
                double tmpNetPrice=getNetPrice(netIDToRange_lock,tmpBreakoutInfo.afterNetID,"d");
                if(tmpNetPrice==0){
                    LOG(ERROR)<<"ERROR:can't find net price.";
                    return;
                }else{
                    DLOG(INFO)<<"netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+
                                "netPrice="+boost::lexical_cast<string>(tmpNetPrice);
                }
                invProcessState="3";
                invBreakoutInfo->upPrice=0;
                invBreakoutInfo->downPrice=tmpNetPrice-2*tickPrice;
                invBreakoutInfo->stopProfitNetID=tmpBreakoutInfo.afterNetID;
                aggOrderInsertStrategy(invBreakoutInfo->initDirection,marketdatainfo);
            }else if(lastPrice<invBreakoutInfo->downPrice){
                DLOG(INFO)<<"停止做市";
                invProcessState="1";
                invBreakoutInfo->upPrice=0;
                invBreakoutInfo->downPrice=0;
                invBreakoutInfo->initDirection="-1";
                return;
                //stopLossLocked();
            }
        }else if(invBreakoutInfo->initDirection=="1"){
            if(lastPrice<=invBreakoutInfo->downPrice){
                DLOG(INFO)<<"判定阶段，价格触及突破线，继续主动做市。";
                double tmpNetPrice=getNetPrice(netIDToRange_lock,tmpBreakoutInfo.afterNetID,"u");
                if(tmpNetPrice==0){
                    LOG(ERROR)<<"ERROR:can't find net price.";
                    return;
                }else{
                    DLOG(INFO)<<"netID="+boost::lexical_cast<string>(tmpBreakoutInfo.afterNetID)+
                                "netPrice="+boost::lexical_cast<string>(tmpNetPrice);
                }
                invProcessState="3";
                invBreakoutInfo->downPrice=0;
                invBreakoutInfo->upPrice=tmpNetPrice+2*tickPrice;
                invBreakoutInfo->stopProfitNetID=tmpBreakoutInfo.afterNetID;
                aggOrderInsertStrategy(invBreakoutInfo->initDirection,marketdatainfo);
            }else if(lastPrice>invBreakoutInfo->upPrice){
                DLOG(INFO)<<"停止做市";
                invProcessState="1";
                invBreakoutInfo->upPrice=0;
                invBreakoutInfo->downPrice=0;
                invBreakoutInfo->initDirection="-1";
                return;
               // stopLossLocked();
              }
        }else{
            DLOG(INFO)<<"unknown direction="+invBreakoutInfo->initDirection;
        }
        if(agglgsTickNums>=30){
            DLOG(INFO)<<"in 30 ticks,not touch broken line,停止做市";
            invProcessState="1";
            invBreakoutInfo->upPrice=0;
            invBreakoutInfo->downPrice=0;
            invBreakoutInfo->initDirection="-1";
        }
    }
}else{
    invProcessState="0";
    DLOG(INFO)<< "invLastPeriodTR="+boost::lexical_cast<string>(invLastPeriodTR)+"<=invtrMetric="+boost::lexical_cast<string>(invTrMetric)+
                 "reset invProcessState="+invProcessState;
}
