# monthly_eos_payment
EOS SmartContract draft for monthly Eos payments and claiming 

NO WARRENTY - USE AT YOUR OWN RISK - NOT AUDITED - JUST A DRAFT 

Payers can define an amount of EOS for monthy *claiming* of a beneficiary. The authorisiation can be revoked or updated by the payer. The payer can withdraw its free funds at any time. 

* Funds can only be claimed one time per month
* The interval is fixed to the current month
* First claiming is possible at the first second of a new month
* Payments can only be transfered in EOS
* You can use any frontend like bloks.io in the contract-section of your contract

# For monthly payers: how to authorize a beneficiary
In order to authorize a monthly payment just...
1) Send funds to the contract 
2) Call *authorize()* - method with the parameter *myaccount* (your accountname) and *beneficiary* for the receiver. Enter *theamount* in **FULL** format like *1.0000 EOS*

You can withdraw your free funds by using the *withdraw()* method. There are only 2 parameters: 1) myaccount (your beneficiary account) and 2) payer (account name of the user). There is now parameter for the amount, because only the user can enter the amount. In order to get your funds you have to do this claim()-method, but it is easier in this way for the user. The payment can only be claimed, it the current balance of the user is >= the claim amount.

# For beneficiary / receivers of payment
If an user with his account name e.g. *payeraccount* already have authorized a monthly payment for you, you are able to *claim* your monthly funds by using the method *claim()*

# For publishes of this contract 
Please call the *init()* action at first to initialize the table *globals*.
