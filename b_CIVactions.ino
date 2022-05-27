/* 
CIV_template - b_CIVactions W.Dilling/DK8RW

Help procedures for the sketch CIV_template

The procedures "CIV_sendCmds()" and "CIV_getProcessAnswers()"
are down at the bottom of this file!

*/

// Radio database procedures ==================================================================

//---------------------------------------------------------------------------------------------
// Handle the ON/OFF state of the radio

void  setRadioOnOff(radioOnOff_t newState) {

  if (newState>RADIO_NDEF)  // just to be on the safe side, since the data is coming from 
    newState=RADIO_NDEF;    // outside of the system

  if (G_radioOn!=newState){
    G_radioOn=newState;

    //!// put your code here, if you want to react on the change of ON/OFF ...
  
    #ifdef debug
      Serial.println(radioOnOffStr[newState]);
    #endif

  }

    //!// or put your code here ...

}

//---------------------------------------------------------------------------------------------
// Handle the RX/TX state of the radio

void setRXTX(uint8_t newState) {

  if (newState>ON)        // just to be on the safe side, since the data is coming from 
    newState=ON;          // outside of the system

  if (G_RXTX!=newState){
    G_RXTX=newState;

    eval_delayMeas;       // debugging: calculate the time since the input trigger changed and print it out
    
    userPTT(newState);    // call into z_userprog.ino ...


    #ifdef debug
      Serial.println(RXTXstr[newState]);
    #endif
  }
  
}

//---------------------------------------------------------------------------------------------
// Handle new frequency information

void  setFrequency(unsigned long newFrequency) {

  if (G_frequency!=newFrequency){
    G_frequency=newFrequency;

    userFrequency(newFrequency);  // call into z_userprog.ino ...

    #ifdef debug
      Serial.println(newFrequency);
    #endif
  }
  
}

#ifdef modmode
//---------------------------------------------------------------------------------------------
// Handle modulation mode and RX Filter setting

void  setModMode(radioModMode_t newModMode, radioFilter_t newRXfilter) {

  if (newModMode==radioModMode_t(0x17)) newModMode = MOD_DV;  // ICOM definition of "DV" is 0x17, mine is 0x09 ...

  if (newModMode>MOD_NDEF)    // just to be on the safe side, since the data is coming from 
    newModMode=MOD_NDEF;      // outside of the system

  if (newRXfilter>FIL3)       // just to be on the safe side, since the data is coming from 
    newRXfilter=FIL_NDEF;     // outside of the system

  if ((G_Mod != newModMode) || (G_RXfilter != newRXfilter)) {
    G_Mod = newModMode;G_RXfilter = newRXfilter;


    //!// put your code here, if you want to react on the change ...

  
    #ifdef debug
      Serial.print ("Mod:  "); Serial.print   (modModeStr[newModMode]);
      Serial.print (" Fil: "); Serial.println (FilStr[newRXfilter]);
    #endif
  }

    //!// or put your code here ...
  
}
#endif

//=============================================================================================
// get the answers from the radio

void  CIV_getProcessAnswers() {

  // if a query request has taken place recently -> wait a bit in order to give the radio time!
  if ( uint16_t(lpCnt-lp_CIVcmdSent) < lp_gapAfterquery ) return;

  CIVresultL = civ.readMsg(civAddr);

  if (CIVresultL.retVal<=CIV_NOK) { //--------------------------------- valid answer received !
                              
    if (CIVresultL.retVal==CIV_OK_DAV) { // Data available ....................................

      setRadioOnOff(RADIO_ON);  // in any case, there was a valid answer -> radio is here !

      // 1. check the "transceive" informations from the radio (i.e. the info which is 
      //    sent by the radio without query) e.g. when turning the VFO knob

      if (CIVresultL.cmd[1]==CIV_C_F_SEND[1]) {   // operating frequency
        setFrequency(CIVresultL.value);
      }
      
      #ifdef modmode
      if (CIVresultL.cmd[1]==CIV_C_MOD_SEND[1]) { // ModMode
        setModMode(radioModMode_t(CIVresultL.datafield[1]),radioFilter_t(CIVresultL.datafield[2]));
      } 
      #endif
          
      // 2. check the answers to the queries initiated by the CIV master !

      // answer to command CIV_C_TX received ...
      if ((CIVresultL.cmd[1]==CIV_C_TX[1]) && 
          (CIVresultL.cmd[2]==CIV_C_TX[2])) { // (this is a 2 Byte command!)
        CIVwaitForAnswer  = false;
        setRXTX(CIVresultL.datafield[1]);  // store it away and do whatever you want with that ...
      }

  //!//
  // put your code here in case you have added querys which shall be ansered by the radio 
  // with data

      // answer to query for operating frequency received ...
      if (CIVresultL.cmd[1]==CIV_C_F_READ[1]) {
        CIVwaitForAnswer  = false;
        setFrequency(CIVresultL.value);
      }

      #ifdef modmode
      // answer to query for ModMode received ...
      if (CIVresultL.cmd[1]==CIV_C_MOD_READ[1]) {
        CIVwaitForAnswer  = false;
        setModMode(radioModMode_t(CIVresultL.datafield[1]),radioFilter_t(CIVresultL.datafield[2]));
      } 
      #endif

    } //  end Data available
    else { // there has been an answer from the radio saying OK or NOK
      CIVwaitForAnswer  = false;

      if (CIVresultL.retVal==CIV_OK) {
        setRadioOnOff(RADIO_ON);  // valid OK answer -> radio is here !
      }
      else {
        // since the IC9700 answers with "NOK" also in the OFF mode -> do not assume, that the radio is ON!
      }

      //!//
      // put your code here, if necessary, i.e. if you want to process the OK/NOK answer from the radio
      // for a specific command
      
    }

  } // end valid answer received
  else { //------------------------------------------------------------ no answer received
    if ( CIVwaitForAnswer &&                                        // still waiting for answer
         (uint16_t(lpCnt - lp_CIVcmdSent) > lp_waitForAnswer)       // -> timeout !
       ) {
       CIVwaitForAnswer = false;
       setRadioOnOff(RADIO_OFF);  // radio is not available, probably off 
    }
  }

}

//=============================================================================================
// send commands to the radio

void  CIV_sendCmds() {

  // do the RXTX poll (independent whether the radio is connected or not) .....................
  // by this, also the radioON/OFF state can be checked
  if (uint16_t(lpCnt-lp_RXTX_sent) > lp_RXTXquery) { // it's time to ask the radio

    if (CIVwaitForAnswer==false) { // ask only, if we currently are not waiting for the radio
      civ.writeMsg (civAddr,CIV_C_TX,CIV_D_NIX,CIV_wFast);
      CIVwaitForAnswer  = true;
      lp_CIVcmdSent     = lpCnt;  // store the time (loopnumber) of the query
      lp_RXTX_sent      = lpCnt; 
    }

  }

  //-------------------------------------------------------------------------------------------
  // the other queries shall take place only if the radio is connected
  if (G_radioOn==RADIO_ON) {


  //!//
  // put your code here in case you add querys...
  // please don't forget: if you add a query command for another radio parameter,
  // you need to add also the same command in the answer section above and a query timer
  // in the main tab!


    // slow poll of the frequency, just to be on the safe side .................................
    if (uint16_t(lpCnt - lp_f_sent) > lp_slowQuery) {  // ask the radio

      if (CIVwaitForAnswer==false) { // ask only, if we currently are not waiting for the radio
        civ.writeMsg (civAddr,CIV_C_F_READ,CIV_D_NIX,CIV_wFast);
        CIVwaitForAnswer  = true;
        lp_CIVcmdSent     = lpCnt;  // store the time (loopnumber) of the query
        lp_f_sent         = lpCnt; 
      }

    }
    #ifdef modmode
    // slow poll of the ModMode, just to be on the safe side ..................................
    if (uint16_t(lpCnt - lp_Mod_sent) > lp_slowQuery) {  // ask the radio

      if (CIVwaitForAnswer==false) { // ask only, if we currently are not waiting for the radio
        civ.writeMsg (civAddr,CIV_C_MOD_READ,CIV_D_NIX,CIV_wFast);
        CIVwaitForAnswer  = true;
        lp_CIVcmdSent     = lpCnt;  // store the time (loopnumber) of the query
        lp_Mod_sent       = lpCnt;
      }

    }
    #endif


  } // if RADIO_ON

}
