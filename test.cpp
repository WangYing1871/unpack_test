//--------------------------------Stamp-------------------------------
//^-^ Author: Zhi Heng            Email: wy187110@mail.ustc.edu.cn     
//^-^ Time: 2023-07-25 10:38:30   Posi: Hefei
//^-^ File: test.cpp
//--------------------------------------------------------------------
#include <algorithm>
#include <numeric>
#include <iostream>
#include <ranges>
#include <exception>
#include <typeinfo>
#include <cxxabi.h>
#include "base.hpp"
#include "channal_map.h"
namespace util{
template <class _tp>
struct type_name{
  typedef std::string result_type;
  result_type operator()(){
    result_type tmp{};
    auto const* type_info = std::__addressof(typeid(_tp));
    if (!type_info) throw std::invalid_argument("...");
    char const* name_aa = type_info->name();
    int status_aa;
    char* name_ab = abi::__cxa_demangle(name_aa,0,0,std::__addressof(status_aa));
    if(name_ab) tmp = name_ab;
    if(status_aa==0) free(name_ab);
    return tmp;
  } };

template <class _tp>
std::string get_tpname(_tp const&){
  return type_name<_tp>{}(); }

}

int main(int argc, char* argv[]){
  typedef std::vector<uint16_t> wave_t;
  channal<wave_t> channal_aa;
  std::iota(std::begin(channal_aa._data_set),std::end(channal_aa._data_set)
      ,(uint16_t)0);

  FEE_board<std::string,channal<wave_t>>* fee_aa = 
    new FEE_board<std::string,channal<wave_t>>{};
  fee_aa->name("fee_aa");
  auto const& call_aa = [&](uint32_t const& _0)mutable->std::string{
    return channal_table_b1[_0]; };
  fee_aa->set_ctb(call_aa);

  //for(auto&& x : channal_table_b1)
  //  std::cout<<std::invoke(fee_aa->channals_table,x.first)<<std::endl;
  for(auto&& x : channal_table_b1)
    fee_aa->add_channal(fee_aa->channals_table(x.first),false);

  //for(auto&& x : channal_table_b1){
  //  fee_aa->add_channal(x.first,fee_aa->channals_table);
  //}

  /*
  for(auto&& x : fee_aa->_channals)
    std::cout<<x.first<<" "<<x.second.first<<" "<<x.second.second.get_tag()<<" "
      <<x.second.second.get_category().name()<<std::endl;
  */

  typedef std::array<uint16_t,16>  wave_t1;
  channal<wave_t1> channal_ab;

  vl_cr_test_device<std::string,512> device_aa;
  auto fee_ab = new FEE_board<std::string,channal<wave_t1>>{};
  for(int i=0; i<16; ++i)
    fee_ab->
      add_channal("fee_ab_APD_channal_"+std::to_string(i),channal<wave_t1>{100+i});
  fee_ab->name("fee_ab");
  for(int i=0; i<6; i+=2){
    std::string name_aa = "Layer_"+std::to_string(i)+"_"+fee_aa->name();
    std::string name_ab = "Layer_"+std::to_string(i)+"_"+fee_ab->name();
    device_aa.add_board(name_aa,fee_aa);
    device_aa.add_board(name_ab,fee_ab);
  }

  for(auto&& x : device_aa.FEE_boards()){
    info_out("----------------------------------------");
    std::cout<<x.second->name()<<std::endl;
    info_out(util::get_tpname(x.second));
    if(x.second->name()=="fee_ab"){
      auto const* aptr = static_cast<FEE_board<std::string,channal<wave_t1>>*>(x.second);
      info_out(aptr->channals_size());
      info_out(util::get_tpname(std::begin(aptr->channals())->second.second._data_set));
      
    }else if(x.second->name()=="fee_aa"){
      //auto const* aptr =  //??? FIXME how to use |dynamic_cast|
      //  dynamic_cast<FEE_board<std::string,channal<wave_t >>*>(x.second);
      auto const* aptr = static_cast<FEE_board<std::string,channal<wave_t >>*>(x.second);
      info_out(aptr->channals_size());
      info_out(util::get_tpname(std::begin(aptr->channals())->second.second._data_set));
    }
  }

  unname_aa events;
  events.set_device(device_aa);
  info_out(events.get_boards());
  

  



  return 0;
}

