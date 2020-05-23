/*
membershipfe.cpp (12.mai.2020 - Beginn)


12.mai.2020 - Beginn
23.mai.2020 - Beta

todo:

*/

#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/transaction.hpp> // tapos...
#include <eosiolib/crypto.h>        // sha...

using namespace eosio;
using namespace std;

#include "membershipfe.hpp"     
#include <ctime>
#include <eosiolib/time.hpp>  






using namespace eosio;
using namespace std;


 
    


const std::string   version   = "V1.0";
 

CONTRACT membershipfe : public eosio::contract {

  public:
      using contract::contract;
      
      // ---
      // Constants
      //
      const uint32_t     hashwert    = 23434;      
      
      
      struct param_struct
      {
      std::string param;    
      };



      struct transfer_args
      {
      name from;
      name to;
      asset quantity;
      std::string memo;
      };    

    
    
      // ---
      // struct global
      //
      struct [[eosio::table]]  global
      {
      uint64_t id; 
      uint64_t cntuser; 
      uint64_t cntappr;
      
      auto primary_key() const { return id; }
      EOSLIB_SERIALIZE(global, (id)(cntuser)(cntappr) )
      }; // struct global      
      
      typedef eosio::multi_index< name("global"), global> globals;
      
      
           
               
     
     // ---
      // struct deposits
      //
      struct [[eosio::table]]  deposits
      {
      uint64_t      id;    
      name          user;            
      uint64_t      amount;       
      
      auto primary_key() const { return id; } 
      EOSLIB_SERIALIZE(deposits, (id)(user)(amount) )
      }; //struct deposits
      
      typedef eosio::multi_index<name("deposits"), deposits> deposit;

 
   
   
      // ---
      // struct approvals
      //
      struct [[eosio::table]]  approvals
      { 
      uint64_t      id;    
      name          user;     
      name          beneficiary;     
      uint128_t     index128;    

      uint64_t      amount;     
      std::string   lastclaim; //  std::string lastclaim;

      int           last_year;
      int           last_month;
      
      auto primary_key() const { return id; } 
      uint128_t by_key() const { return index128; }      
      EOSLIB_SERIALIZE(approvals, (id)(user)(beneficiary)(index128)(amount)(lastclaim)(last_year)(last_month))

      }; //struct deposits
        
      typedef eosio::multi_index<name("approvals"), approvals ,    indexed_by<name("bykey"), const_mem_fun<approvals, uint128_t, &approvals::by_key>>          > approval;
  
     
     
     
      // Create u128 from too u64 values
      #define LIT128(HI,LO) ((((__uint128_t) HI) << 64) | LO)

                        
    
 
    
      inline void splitMemo(std::vector<std::string> &results, std::string memo)
            {
            auto end = memo.cend();
            auto start = memo.cbegin();

            for (auto it = memo.cbegin(); it != end; ++it)
                {
                if (*it == ';')
                   {
                   results.emplace_back(start, it);
                   start = it + 1;
                   }
                }
            if (start != end)
            results.emplace_back(start, end);
            } // splitMemo


  
      // ---
      // send_token - sending token.
      //  
      inline void send_token( name receiver, uint64_t amount, std::string memo) 
      {                   
      asset quant = asset(amount, symbol("EOS", 4)  );            
      action(
            permission_level{_self, name("active")},
            name("eosio.token"), name("transfer"),
            std::make_tuple(_self, receiver, quant, memo )
            ).send();
      } // void send_token()

  


      // ---
      // handle_transfer - xxx.
      //
      [[eosio::action]]  
      void hndltransfer()
      {
      
      auto data = unpack_action_data<transfer_args>();

           
      if ( data.from == get_self()) 
         {
         print(" DO NOTHING ");
         return;
         }
      

      eosio_assert(
                  has_auth( get_self() ) || 
                  has_auth( data.from 
                  )  
                  ,
                  " missing required authority!!! ");
      
      std::vector<std::string> results;
      splitMemo(results, data.memo);
       
     
      // Only EOS
      eosio_assert( data.quantity.symbol == symbol("EOS" , 4), " Wrong Currency ");  
      
      
            
 //   if (results[0] == "deposit")            
         {
         require_auth( data.from );
              
         //
         // Get globals
         //
         int id = 0;
         globals myglobals(_self, _self.value);
         auto iterator_globals = myglobals.find(id);

         uint64_t cntuser = iterator_globals->cntuser;        
         cntuser++;
               
         
         deposit mydeposit( _self , _self.value );    
         auto iterator = mydeposit.find( data.from.value );
           
         uint64_t      amount_old =   iterator->amount;
         print(" amount_old: ",amount_old, " ");
              
         if ( iterator != mydeposit.end() )
            {
            // User exists
            
            mydeposit.modify(iterator, _self, [&](auto& tupel) 
                   {                                                                            
                   tupel.amount     = data.quantity.amount + amount_old;              
                   });     
                   
            } else
              {
              // User will be created... 
               
              mydeposit.emplace(_self, [&](auto& tupel) 
                 {
                 tupel.id         = data.from.value;
                 tupel.user       = data.from;
                 tupel.amount     = data.quantity.amount;
                 });
   
              // ---  
              // Finaly update globals
              //
              if ( iterator_globals != myglobals.end() )
                 {                        
                 myglobals.modify(iterator_globals, _self, [&](auto& global) 
                    {                                                    
                    global.cntuser           = cntuser;                                                            
                    });            
                 } // if...    
                  
              } // else
             
                  
         } 
        // else print( " NO NO NO ");
            
      
      } // handle_transfer
    
    
    
      // -------------------------------------
      // INIT - Call THIS funktion FIRST 
      // -------------------------------------
      [[eosio::action]]  
      void init() 
      {    
      require_auth(get_self());
            
      // ---
      // Globals
      //
      int id = 0;
      globals myglobals( _self , _self.value );         
      auto iterator = myglobals.find(id);
    
      if ( iterator != myglobals.end() )
         {
         print(" globals does exist");
         } else
              {
              print(" globals will be created... ");
              myglobals.emplace(_self, [&](auto& global) 
                 {
                 global.id = id;
                 global.cntuser        = 3;
                 });
              } // else
                        
      } // init()     
      
     
     
     
     
      
       
      // ---
      // reset - remove globals
      //            
      [[eosio::action]]  
      void reset() 
      {   
      require_auth(get_self());
            
      globals myglobals(_self, _self.value);
         
      for (auto itr = myglobals.begin(); itr != myglobals.end();) 
          {
          itr = myglobals.erase(itr);
          }              
      } // reset()                 



     
            
            
            
            
      // ---
      // withdraw - withdraw own funds
      //
      [[eosio::action]]  
      void withdraw( name myaccount, asset theamount  ) 
      {
      require_auth( myaccount );
      
      uint64_t wd_amount = theamount.amount;
       
      deposit mydeposit( _self , _self.value );            
      auto iterator = mydeposit.find( myaccount.value );
           
           
      uint64_t      amount_old =   iterator->amount;
            
              
      if ( iterator != mydeposit.end() )
            {          
            eosio_assert( wd_amount <= amount_old  , " Balance to low! ");
            eosio_assert( wd_amount > 0  , " Withdraw must bigger than zero! ");
         
            // Send withdraw balance
            send_token( myaccount, wd_amount, " Withdraw from contract ")   ;   
           
            mydeposit.modify(iterator, _self, [&](auto& tupel) 
                   {                                                                     
                   tupel.amount     = amount_old - wd_amount;                                                                            
                   });     
   
            } else
              {
              int ERROR = 1;
              eosio_assert( ERROR == 0 , " USER NOT FOUND ");    
              } // else

      }  // withdraw
      
           
           
           
      //     
      // authorize()
      // 0 eos            -> delete entry
      // if data existing -> only modify
      // 
      [[eosio::action]]  
      void authorize( name myaccount, name beneficiary, asset theamount  ) 
      {
      require_auth( myaccount );
                  
      eosio_assert( myaccount != beneficiary  , " Same account is not allowed! "); 
       
            

      // Create index            
      uint128_t index128 = LIT128(myaccount.value, beneficiary.value);
     
      
       //
       // Get globals
       //
       int id = 0;
       globals myglobals(_self, _self.value);
       auto iterator_globals = myglobals.find(id);

       uint64_t cntappr = iterator_globals->cntappr;      
       cntappr++;
       
       
 
    
         
       // --- 
       // is d_depositid still registered?
       //
       approval myapprovals(_self, _self.value);
                                      
       auto zip_index = myapprovals.get_index<name("bykey")>();     
       auto iterator_appr = zip_index.find( index128 );
  
       if ( iterator_appr->index128 == index128 )
            {            
            print(" INDEX FOUND! id:", iterator_appr->id, " ");
           
           

            // Delete if amount = 0
            if (theamount.amount == 0)           
               {
               
               //
               // Get iterator
               //           
               approval myapprovals2(_self, _self.value);
               auto iterator_appr2 = myapprovals2.find( iterator_appr->id );
                          
               myapprovals2.erase( iterator_appr2 ); 
      
               }
            else
            {   
            //
            // Get iterator (classic way)
            //           
            approval myapprovals2(_self, _self.value);
            auto iterator_appr2 = myapprovals2.find( iterator_appr->id );
            
            myapprovals2.modify(iterator_appr2, _self, [&](auto& tupel) 
                   {                                                                                     
                   tupel.amount        = theamount.amount;                
                   tupel.last_year     = -1;                                
                   tupel.last_month    = -1;                                                                                             
                   });     
            } // else    
                             
            } else 
              {
              // INDEX NOT FOUND! 
               
              
                myapprovals.emplace(_self, [&](auto&  tupel) 
                {                
                tupel.id            = cntappr;                
                tupel.user          = myaccount;
                tupel.beneficiary   = beneficiary;                
                tupel.index128      = index128;                
                tupel.amount        = theamount.amount;                
                tupel.lastclaim     = "";
                tupel.last_year     = -1;                                
                tupel.last_month    = -1;                                                                
                }); 
                

              //
              // Update globals
              //                                
              myglobals.modify(iterator_globals, _self, [&](auto& global) 
                   {                                                                               
                   global.cntappr       = cntappr;                                                                                                
                   });                          
                
              }
            
 
      } // authorize
      
      
     
      
      
      
      
      //
      // claim
      //
      [[eosio::action]]  
      void claim( name myaccount, name payer ) 
      {
      require_auth( myaccount );
      
      // Create index      
      uint128_t index128 = LIT128(payer.value, myaccount.value);
      
      
   
      auto thetime = now();
       
      int p_year;
      int p_month;
      int p_day;
      int p_hour;
      int p_minute; 
      int p_second;     
      
      convert_ts(  (unsigned long int) thetime,
                           &p_year, &p_month, &p_day,
                           &p_hour, &p_minute, &p_second);
 
 
      
      
       // --- 
       // is d_depositid still registered?
       //
       approval myapprovals(_self, _self.value);
                                     
       auto zip_index = myapprovals.get_index<name("bykey")>();     
       auto iterator_appr = zip_index.find( index128 );
  
       // If approval found
       if ( iterator_appr->index128 == index128 )
            {                        
            
            // ---
            // Get deposits of payer
            //
            deposit mydeposit( _self , _self.value );            
            auto iterator = mydeposit.find( payer.value );                      
            uint64_t      amount_payer =   iterator->amount;
     
            
            // ---
            // Enough blance?
            //
            if ( iterator_appr->amount <= amount_payer )
               {
               // Payer has enough amount               
                 
               int ERROR = 0;
                              
               if ( 
                  ( iterator_appr->last_year == p_year ) && ( iterator_appr->last_month == p_month )
                  ) ERROR = 1;
                              
               eosio_assert( ERROR == 0, " ALREADY PAYED! ");  
               
               // ---
               // Create memo
               //
               std::string str_year   =  std::to_string(  p_year  );  
               std::string str_month  =  std::to_string(  p_month  );  
               std::string str_day    =  std::to_string(  p_day  );  
               std::string str_hour   =  std::to_string(  p_hour );  
               std::string str_minute =  std::to_string(  p_minute );  
               std::string str_second =  std::to_string(  p_second  );  
             
               std::string memo_date = str_year + "-" + str_month + "-" + str_day + " " + str_hour + ":" + str_minute + ":" + str_second;
               std::string memo      = "Claim " + memo_date;

               // 
               // END - date memo
               // ---
               
               
               // ---               
               // Send withdraw balance
               send_token( myaccount, iterator_appr->amount, memo )   ;                
               

               // ---                                 
               // Update Deposit
               mydeposit.modify(iterator, _self, [&](auto& tupel) 
                   {                                                                            
                   tupel.amount     = amount_payer - iterator_appr->amount;
                   });     
               
               // ---    
               // Update apporval
               
                  //
                  // Get iterator (classic way)
                  //           
                  approval myapprovals2(_self, _self.value);
                  auto iterator_appr2 = myapprovals2.find( iterator_appr->id );
            
                  myapprovals2.modify(iterator_appr2, _self, [&](auto& tupel) 
                    {                                                                      
                    tupel.lastclaim     = memo_date;
                    tupel.last_year     = p_year;                                
                    tupel.last_month    = p_month;                                                                                             
                    });    

               print(" PAYED! ");
               
               } else
                 {
                 int ERROR = 1;
                 eosio_assert( ERROR == 0 , " PAYER HAS NOT ENOUGH AMOUNT! ");   
                 } 

            
         
            } else
              {
              print(" AUTORISATION NOT FOUND! ");
              int ERROR = 1;
              eosio_assert( ERROR == 0 , " AUTORISATION NOT FOUND! "); 
              }
           

      } // claim
      
      
      

void convert_ts(unsigned long int unixtime,
                           int *p_year, int *p_month, int *p_day,
                           int *p_hour, int *p_minute, int *p_second)
{
    const unsigned long int SECONDS_PER_DAY    =  86400ul; 
    const unsigned long int DAYS_IN_YEAR       =    365ul; 
    const unsigned long int DAYS_IN_4_YEARS    =   1461ul; 
    const unsigned long int DAYS_IN_100_YEARS  =  36524ul; 
    const unsigned long int DAYS_IN_400_YEARS  = 146097ul; 
    const unsigned long int DAYN_AD_1970_01_01 = 719468ul; 

    unsigned long int DayN = DAYN_AD_1970_01_01 + unixtime/SECONDS_PER_DAY;
    unsigned long int seconds_since_midnight   = unixtime%SECONDS_PER_DAY;
    unsigned long int temp;

    temp = 4 * (DayN + DAYS_IN_4_YEARS + 1) / DAYS_IN_400_YEARS - 1;
    *p_year = 100 * temp;
    DayN -= DAYS_IN_100_YEARS * temp + temp / 4;
   
    temp = 4 * (DayN + DAYS_IN_YEAR + 1) / DAYS_IN_4_YEARS - 1;
    *p_year += temp;
    DayN -= DAYS_IN_YEAR * temp + temp / 4;

    *p_month = (5 * DayN + 2) / 153;
    *p_day = DayN - (*p_month * 153 + 2) / 5 + 1;
     
    *p_month += 3; 
    
    if (*p_month > 12)
    {       
        *p_month -= 12;
        ++*p_year;
    }

    *p_hour    = seconds_since_midnight / 3600;
    *p_minute  = seconds_since_midnight % 3600 / 60;
    *p_second  = seconds_since_midnight % 60;
} // convert_ts




      
     
      
      
       
      
      
      
      
      
}; // CONTRACT membershipfe      




extern "C"
{

void apply(uint64_t receiver, uint64_t code, uint64_t action) 
{
 

       
       
    if (
       (code == name("eosio.token").value) &&
       action == name("transfer").value
       )
       {       
       execute_action(name(receiver), name(code), &membershipfe::hndltransfer);  
       }

   if (action == name("init").value)
       {
       execute_action(name(receiver), name(code), &membershipfe::init);  
       }
              
   if (action == name("reset").value)
       {
       execute_action(name(receiver), name(code), &membershipfe::reset);  
       }   
                             
    if (action == name("withdraw").value)
       {
       execute_action(name(receiver), name(code), &membershipfe::withdraw);  
       }            

    if (action == name("authorize").value)
       {
       execute_action(name(receiver), name(code), &membershipfe::authorize);  
       }            

    if (action == name("claim").value)
       {
       execute_action(name(receiver), name(code), &membershipfe::claim);  
       }            
       
       
             


 
    


   

} // apply
    
    
    
       
} // extern "C"      

