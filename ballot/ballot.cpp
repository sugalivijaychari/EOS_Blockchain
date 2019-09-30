#include <eosio/eosio.hpp>

using namespace eosio;

class [[eosio::contract("ballot")]] ballot : public eosio::contract{
public:
  ballot(name receiver, name code,  datastream<const char*> ds): contract(receiver, code, ds) {}

  [[eosio::action]]
  void setowner(name user, uint64_t length){
    require_auth(user);
    chair_index chairs(get_first_receiver(),get_first_receiver().value);
    auto iterator = chairs.find(user.value);
    if(iterator == chairs.end()){
      chairs.emplace(user, [&](auto& row){
        row.ceo = user;
        row.c_length = length;
      });
      print(user," is set as CEO to conduct ballot elections. Now, ", user, " he can add contestants.");
      print("No. of contestants in ballot election is: ", length);
      voter_index voters(get_first_receiver(),get_first_receiver().value);
      auto v_itr = voters.find(user.value);
      if(v_itr == voters.end()){
        voters.emplace(user, [&](auto& row){
          row.voter_name = user;
          row.weight = 1;
          row.voted = false;
          row.vote = "";
        });
        print("Dear ", user, " , you got vote power.");
      }else{
        print("Dear ", user, " , you already got vote power.");
      }
    }else{
      print("CEO was already set for this ballot elections.");
    }
  }

  [[eosio::action]]
  void adcontestant(name user, std::string c_name){
    require_auth(user);
    chair_index chairs(get_first_receiver(), get_first_receiver().value);
    auto& _chair = chairs.get(user.value, "You are not authorized to add a contestant.");
    uint64_t limit = _chair.c_length;
    contestant_index contestants(get_first_receiver(),get_first_receiver().value);
    uint64_t c_count = 0;
    for(auto& item : contestants){
      c_count++;
    }
    if(c_count+1<=limit){
      name cname = name(c_name);
      auto c_itr = contestants.find(cname.value);
      if(c_itr == contestants.end()){
        contestants.emplace(user, [&](auto& row){
          row.c_name = cname;
          row.votescount = 0;
        });
        print("Contestant ", cname, " is added into ballot elections.");
      }else{
        print("Contestant ", cname, " was already added into ballot elections.");
      }
    }else{
      print("You can't add more contestants than the limit.");
    }
  }

  [[eosio::action]]
  void addvoter(name user, std::string v_name){
    require_auth(user);
    voter_index voters(get_first_receiver(), get_first_receiver().value);
    name vname = name(v_name);
    auto v_itr = voters.find(vname.value);
    if(v_itr == voters.end()){
      voters.emplace(user, [&](auto& row){
        row.voter_name = vname;
        row.weight = 1;
        row.voted = false;
        row.vote = "";
      });
      print("Voter ",vname," got power to vote.");
    }else{
      print("Voter ",vname," had already got power to vote.");
    }
  }

  [[eosio::action]]
  void vote(name user, std::string to){
    require_auth(user);
    name cname = name(to);
    contestant_index contestants(get_first_receiver(),get_first_receiver().value);
    auto c_itr = contestants.find(cname.value);
    if(c_itr == contestants.end()){
      print("The contestant ",cname," is no longer a participant in this ballot elections.");
    }else{
      contestants.modify(c_itr, user, [&](auto& row){
        uint64_t p_count = row.votescount;
        row.votescount = p_count +1;
      });
    }
  }

  [[eosio::action]]
  void winner(name user){
    require_auth(user);
    contestant_index contestants(get_first_receiver(), get_first_receiver().value);
    uint64_t max = 0;
    std::string winner;
    for(auto& item:contestants){
      if(item.votescount > max){
        max = item.votescount;
        auto n = name{item.c_name};
        winner = n.to_string();
      }
    }
    print("The contestant - ", winner," won the ballot elections with ", max, " votes.");
  }


private:

  struct [[eosio::table]] chair{
    name ceo;
    uint64_t c_length;

    uint64_t primary_key() const{
      return ceo.value;
    }
  };

  struct [[eosio::table]] voter{
    name voter_name;
    uint64_t weight;
    bool voted;
    std::string vote;

    uint64_t primary_key() const{
      return voter_name.value;
    }
  };

  struct [[eosio::table]] contestant{
    name c_name;
    uint64_t votescount;

    uint64_t primary_key() const{
      return c_name.value;
    }
  };

  typedef eosio::multi_index<"chairs"_n, chair> chair_index;
  typedef eosio::multi_index<"voters"_n, voter> voter_index;
  typedef eosio::multi_index<"contestants"_n, contestant> contestant_index;

};
