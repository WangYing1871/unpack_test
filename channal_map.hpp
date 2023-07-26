#ifndef channal_map_H
#define channal_map_H 1 
#include <map>
#include <string>

typedef int channal_index_t;
static std::map<channal_index_t,std::string>
  channal_table = {
  {9,"LH10"} ,{10,"HL0"} ,{11,"LH5"} ,{12,"HH20"}
  ,{13,"HL5"} ,{14,"HH15"} ,{15,"LL15"} ,{16,"LL5"}
  ,{17,"LH21"} ,{18,"LH20"} ,{0,"HH21"} ,{19,"HL20"}
  ,{20,"HL15"} ,{21,"LH15"} ,{22,"LL21"} ,{23,"LH1"}
  ,{24,"HL16"} ,{25,"HH1"} ,{26,"HH11"} ,{27,"LL11"}
  ,{28,"LL1"} ,{1,"HL1"} ,{29,"LH16"} ,{30,"HL6"}
  ,{31,"HH16"} ,{32,"HL11"} ,{33,"LH11"} ,{34,"LL16"}
  ,{35,"LH6"} ,{36,"HL17"} ,{37,"HH6"} ,{38,"LH12"}
  ,{2,"HH10"} ,{39,"HL12"} ,{40,"LL6"} ,{41,"LH17"}
  ,{42,"HL2"} ,{43,"HH17"} ,{44,"LL12"} ,{45,"HH12"}
  ,{46,"LL17"} ,{47,"LH2"} ,{48,"HH2"} ,{3,"LL10"}
  ,{49,"HL22"} ,{50,"LL18"} ,{51,"HH18"} ,{52,"LH22"}
  ,{53,"LL2"} ,{54,"HH22"} ,{55,"HL7"} ,{56,"LH18"}
  ,{57,"HL18"} ,{58,"LH7"} ,{4,"LH0"} ,{59,"LL22"}
  ,{60,"HH7"} ,{61,"HL23"} ,{62,"LL13"} ,{63,"HH13"}
  ,{64,"LH23"} ,{65,"LL7"} ,{66,"HH23"} ,{67,"HL3"}
  ,{68,"LH13"} ,{5,"HH0"} ,{69,"HL13"} ,{70,"LH3"}
  ,{71,"LL23"} ,{72,"LH24"} ,{73,"HH4"} ,{74,"LL14"}
  ,{75,"HH14"} ,{76,"LH19"} ,{77,"LL3"} ,{78,"HL4"}
  ,{6,"HH5"} ,{79,"HH8"} ,{80,"LH14"} ,{81,"HL14"}
  ,{82,"LH8"} ,{83,"LL19"} ,{84,"HL8"} ,{85,"HH19"}
  ,{86,"HL9"} ,{87,"LH9"} ,{88,"HH24"} ,{7,"HL21"}
  ,{89,"LL8"} ,{90,"HL19"} ,{91,"HH3"} ,{92,"HH9"}
  ,{93,"LL9"} ,{94,"LH4"} ,{95,"HL24"} ,{8,"HL10"} };

namespace util{
static constexpr std::index_sequence<4,8> LC_idx;
}

namespace util{

template <class _tp, template <class, class...> class _cont_tt=std::vector
  ,template <class...> class _tup_t=std::tuple, class... _args>
_cont_tt<_tp> unpack(_tup_t<_args...> const& _p0){
  constexpr std::size_t NN = sizeof...(_args);
  _cont_tt<_tp> tmp(NN);
  [&]<std::size_t... I>(std::index_sequence<I...>){
    (...,(tmp.at(I) = static_cast<_tp>(std::get<I>(_p0))));
  }(std::make_index_sequence<NN>()); 
  return tmp; }

template <std::size_t N>
struct size_{
  constexpr std::size_t static value = N; };

template <std::size_t N, std::size_t M, std::size_t value_first, std::size_t... _values>
struct get_helper{
  constexpr std::size_t static value = get_helper<N,M+1,_values...>::value; };
template <std::size_t N, std::size_t value_first, std::size_t... _values>
struct get_helper<N,N,value_first,_values...>{
  constexpr std::size_t static value = value_first; };

template <std::size_t N, class...>
struct get;

template <std::size_t N, std::size_t... _values>
requires (N<=(sizeof...(_values)-1))
struct get<N,std::index_sequence<_values...>>{
  constexpr static std::size_t value = get_helper<N,0,_values...>::value;};

template <std::size_t N, std::size_t... _values>
requires (N<=(sizeof...(_values)-1))
struct get_c{
  constexpr static std::size_t value = get_helper<N,0,_values...>::value;};

template <std::size_t _index_l, std::size_t _index_u, std::size_t... _values>
requires (_index_l>=0 && _index_u<=sizeof...(_values) && _index_l<=_index_u)
struct range_sum_helper{
  static constexpr std::size_t value = get_c<_index_l,_values...>::value
    + range_sum_helper<_index_l+1,_index_u,_values...>::value; };
template <std::size_t _index_l, std::size_t... _values>
struct range_sum_helper<_index_l,_index_l,_values...>{
  static constexpr std::size_t value = 0UL; };

//I don't know(or don't want) to unify 'range_sum' and 'range_sum_c', so please make do with it;
// so as 'get'
//I don't have the energy to talk about those unified algorithms, maybe you can seek help from "boost::mpl" and "boost::mp11"
template <std::size_t,std::size_t,class... _args>
struct range_sum;

template <std::size_t _index_l, std::size_t _index_u, std::size_t... _values>
struct range_sum<_index_l,_index_u,std::index_sequence<_values...>>{
  static constexpr std::size_t value = 
    range_sum_helper<_index_l,_index_u,_values...>::value; };

template <std::size_t _index_l,std::size_t _index_u,std::size_t... _values>
struct range_sum_c{
  static constexpr std::size_t value = 
    range_sum_helper<_index_l,_index_u,_values...>::value; };

template <class _tp, class _up=_tp, class... _args, std::size_t... _values>
requires std::unsigned_integral<_tp> && (sizeof...(_args)>=sizeof...(_values))
void encode(_tp& _dest,std::index_sequence<_values...>
    ,_args&&... _codes){
  _dest = (std::numeric_limits<_tp>::min)();
  constexpr static std::size_t NN = sizeof...(_values);
  auto codes = unpack<_up>(std::make_tuple(std::forward<_args>(_codes)...));
  [&]<std::size_t... I>(std::index_sequence<I...>){
    (...,(_dest |= codes.at(I) <<(range_sum_c<I+1,NN,_values...>::value)));
  }(std::make_index_sequence<NN>{}); }

template <class _tp, std::size_t N>
requires (std::unsigned_integral<_tp> && N<=__CHAR_BIT__*sizeof(_tp))
_tp decode_helper(_tp const& _p0){
  _tp bs = (std::numeric_limits<_tp>::max)();
  bs >>= sizeof(_tp)*__CHAR_BIT__-N;
  return _p0 & bs; }
template<class _tp, class _up=_tp, std::size_t... _values>
std::array<_up,sizeof...(_values)> decode(_tp const& _from, std::index_sequence<_values...>){
  constexpr static std::size_t NN = sizeof...(_values);
  std::array<_up,NN> tmp{};
  [&]<std::size_t... I>(std::index_sequence<I...>){
    (...,(tmp.at(NN-I-1)=decode_helper<_up,get_c<I,_values...>::value>(static_cast<_up>(_from>>range_sum_c<I+1,NN,_values...>::value))));
  }(std::make_index_sequence<NN>{});
  return tmp; }
}
#endif
