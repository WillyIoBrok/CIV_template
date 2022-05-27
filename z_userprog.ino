/* 
CIV_template - z_userprog

This is the part, where the user can put his own code

The calls to this user programs shall be inserted wherever it suits - search for //!//
in all files

*/

//=========================================================================================
// user part of the defines



//=========================================================================================
// user part of the database



//=========================================================================================
// this is called, when the RX/TX state changes ...
void  userPTT(uint8_t newState) {
  
}

//=========================================================================================
// this is called, whenever there is new frequency information ...
void userFrequency(unsigned long newFrequency) {

}

//=========================================================================================
// this will be called in the setup after startup
void  userSetup(){
  
}

//-------------------------------------------------------------------------------------
// this will be called in the baseloop every BASELOOP_TICK[ms]
void  userBaseLoop() {
  
}
