#include <eosio/eosio.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract("inframind")]] inframind : public contract {
public:
  inframind(name receiver, name code, datastream<const char*> ds) : contract(receiver, code, ds){}

  [[eosio::action]]
  void bankregister(name bname, string dno, string adline1, string adline2, string city, string province, string country, string mailid, string website, uint64_t pincode){
    require_auth(bname);
    bank_index banks(get_first_receiver(), get_first_receiver().value);
    auto iterator = banks.find(user.value);
    if(iterator == banks.end()){
      banks.emplace(bname, [&](auto& row){
        row.b_name = bname;
        auto size = std::count(banks.cbegin(),banks.cend());
        row.b_id = size + 1;
        row.b_address.dno =
      });
    }
  }

private:

  enum occupation: uint8_t {
    OTHERS = 0,
    SALARIED = 1,
    SELFEMPLOYED = 2
  };

  enum stakeholder : uint8_t {
    P_DEALER = 1,
    INS_PROVIDER = 2,
    G_PASSPORT = 3,
    G_VOTERCARD = 4,
    G_LICENSE = 5,
    G_RDO = 6,
    P_COMPANY = 7,
    GP_OTHERS = 8
  };

  enum documents : uint8_t {
    D_PASSPORT = 1,
    D_VOTERID = 2,
    D_LICENSE = 3,
    D_SALARYSLIP = 4,
    D_COMPANYID = 5
  };

  enum status : uint8_t {
    PROPOSED = 1,
    OPENED = 2,
    PAUSED = 3, // If scheme is resumed, then it comes under opened itself.
    CLOSED = 4
  };

  struct address{
    string dno;
    string addressline1;
    string addressline2;
    string city;
    string province;
    string country;
    uint64_t pincode;
    string mailid;
    string website;
  };

  struct approvalinfo{
    uint64_t approvedby;
    time approved_at;
  };

  struct passport{
    string pp_id;
    bool trusted;
    approvalinfo pp_apinfo;
  };

  struct votercard{
    string vl_id;
    bool trusted;
    approvalinfo vl_apinfo;
  };

  struct license{
    string l_id;
    bool trusted;
    approvalinfo l_apinfo;
  }

  struct [[eosio::table]] person{
    name p_name;
    uint64_t p_age;
    string p_dob;
    uint8_t p_occ;
    passport p_passport;
    votercard p_votercard;
    license p_license;

    uint64_t primary_key() const{
      return p_name.value;
    }
  };

  struct [[eosio::table]] customer{
    name c_name;
    uint64_t c_accno;
    string branch;
    bool trusted;
    // Scheme details - vector

    uint64_t primary_key() const{
      return c_name.value;
    }
  };

  struct employee{
    name e_name;
    uint64_t e_id;
  };

  struct shaccord{
    string sender;
    string receiver;
    string memo; // duty of the stake holder to be done in order to be part of the platform
    time req_time;
    time appr_time;
    bool approved;
  };

  struct eligibility{
    vector<uint8_t> occ; // occupations
    uint64_t min_age;
    uint64_t max_age;
    bool earning;
  };

  struct termscond{
    uint64_t process_fee; // processing fee in percentage
    uint64_t min_ldp; // min loan disbursal period in months;
    uint64_t max_ldp; // max
    uint64_t interestrate;
    uint64_t creditscore; // min credit score required.
    bool insuranced;
  };

  struct proofs{
    vector<uint8_t> identity; // list of documents as proof for the context of identity
    vector<uint8_t> adress; // list of documents as proof for the context of address
    vector<uint8_t> income; // list of documents as proof for the context of income
  };

  struct [[eosio::table]] scheme{
    name bname;
    string s_name;
    uint64_t s_id;
    eligibility s_eligible; // To be part of this scheme to take loan, customer should satisfy the eligibility declared by bank
    termscond s_termscond; // To be part of this scheme to take loan, customer should agree the terms and conditions declared by bank
    proofs s_proofs; // To be part of this scheme to take loan, customer should have these documents declared by bank
    vector<string> features; // Features of the scheme can mention here.

    uint64_t primary_key() const{
      return bname.value;
    }
  };

  struct schemestatus{
    uint8_t s_status;
    time proposed;
    time opened;
    vector<time> paused;
    vector<time> resumed;
    time closed;
  };

  struct [[eosio::table]] insurance{
    string cx_name; // name of the customer who has taken loan
    string ins_source; // name of the person or org who provided insurance
    uint64_t ins_id;
    approvalinfo ins_apinfo;

    uint64_t primary_key() const{
      return ins_id;
    }
  };

  struct loanreq{
    uint64_t loanid;
    string customername;
    string dealername;
    string type; // type of the loan. (sub types in home, two wheeler and commercial vehicle loans)
    bool insuranced;
    insurance ins_info;
    uint64_t amount;
    bool eligible;
    bool tc_accepted; // terms and conditions accepted
    bool proofsed; // all the documents are verified
    proofs proofs_info;
    bool req_status; // false - rejected and true - accepted
  };

  struct loanappr{
    uint64_t loanid;
    string customername;
    uint64_t amountpaid;
    string pymt_medium;
    time paid_time;
    string appr_officer; // name of the employee of the bank who approved the loan and paid loan amount
    uint64_t interestrate; // rate of interest in percentage
    uint64_t months; // how many months does a customer pay installments amount
    uint64_t penalty; // amount to be paid by customer if misses to pay installment
    bool appr_status; // false - not yet completed and true - completed
  };

  struct penalty{
    uint64_t p_id;
    uint64_t amount;
    time p_time;
    string emp_name; // employee who monitored
  };

  struct txn{
    string cx_name; //customer name
    uint64_t t_id;
    uint64_t amount;
    time t_time;
    bool tgt_achieved; // target achieved
    bool penaltied;
    uint64_t penalty_id;
    string paid_medium;
    string emp_name; // who has done this transaction
  };

  struct disbursal{
    uint64_t amount; // monthly installment amount
    uint64_t left_emis; // how many installments left
    uint64_t total_due;
    bool penaltied;
    vector<penalty> penalties;
    vector<txn> txns;
  };

  struct ongoing{
    uint64_t loanid;
    string cx_name;
    uint64_t loanamount;
    uint64_t cum_amount; // amount should pay / being paid by customer through the scheme with all expenses
    // loan disbursals
    disbursal disbursals;
    bool cleared;
  };

  struct loan{
    uint64_t loanid;
    // requests
    vector<loanreq> requests;
    // approved
    vector<loanappr> appr_loans;
    // ongoing Loans
    vector<ongoing> active_loans;
    // closed loans
  };

  struct bankscheme{
    uint64_t sid; // The unique id of the schema of the scheme.
    bool qualified; // In order to open the scheme in this platform, government agencies should agree to be part of this for documents verification.
    schemestatus bs_status; // Bank scheme status
    //Loans
    vector<laon> loans;
  };

  struct [[eosio::table]] bank{
    name b_name;
    string b_ceo;
    uint64_t b_id;
    address b_address;
    vector<name> b_customers;
    vector<employee> b_employees;
    vector<uint8_t> b_trusthol; // trusted stake holders
    vector<shaccord> b_shaccords; // stake holder accords(agreements)
    vector<bankscheme> b_schemes;

    uint64_t primary_key() const{
      return b_name.value;
    }
  };

  struct activity{
    name b_name; // bank name
    // Scheme details - vector
  };

  struct [[eosio::table]] dealer{
    name d_name;
    approvalinfo d_apinfo;
    bool trusted;
    vector<activity> activities;

    uint64_t primary_key() const{
      return d_name.value;
    }
  };

  struct [[eosio::table]] insorg{
    name i_name;
    string i_ceo;
    uint64_t i_id;
    address i_address;
    bool trusted;
    vector<employee> i_employees;
    // insurance details

    uint64_t primary_key() const{
      return i_name.value;
    }
  };

  struct [[eosio::table]] department{
    name s_name;
    string s_ceo;
    uint8_t s_type;
    address s_address;
    bool trusted;
    vector<employee> d_employees;
    // verifications - vector.
  };

  struct [[eosio::table]] event{
    uint64_t eid;
    string type;
    string memo;
    time event_time;
    vector<string> stakeholders;
  };

  struct [[eosio::table]] publictxn{
    uint64_t txn_id;
    string bname;
    uint64_t loanid;
    string loantype;
    string cx_name;
    uint64_t paidamount;
    string paidmedium;
    string emp_name;
    time paidtime;
    uint64_t emisleft;
    uint64_t totaldue;
    bool penaltied;
    penalty penaltyinfo;
  };

  typedef eosio::multi_index<"people"_n, person> person_index;
  typedef eosio::multi_index<"customers"_n, customer> customer_index;
  typedef eosio::multi_index<"schemes"_n, scheme> scheme_index;
  typedef eosio::multi_index<"insurances"_n, insurance> insurance_index;
  typedef eosio::multi_index<"banks"_n, bank> bank_index;
  typedef eosio::multi_index<"dealers"_n, dealer> dealer_index;
  typedef eosio::multi_index<"insorgs"_n, insorg> insorg_index;
  typedef eosio::multi_index<"departments"_n, department> department_index;
  typedef eosio::multi_index<"events"_n, event> event_index;
  typedef eosio::multi_index<"publictxns", publictxn> publictxn_index;

};
