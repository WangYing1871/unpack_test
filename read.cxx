//--------------------------------Stamp-------------------------------
//^-^ Author: Zhi Heng            Email: wy187110@mail.ustc.edu.cn     
//^-^ Time: 2023-07-06 15:19:21   Posi: Hefei
//^-^ File: read.cxx
//--------------------------------------------------------------------
#define info_out(X) std::cout<<"==> "<<__LINE__<<" "<<#X<<" |"<<(X)<<"|\n"
#include <bits/c++config.h>
#include <array>
#include <numeric>
#include <algorithm>
#include <functional>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <fstream>
#include <exception>
#include <list>
#include <filesystem>
#include <iterator>
#include <map>
#include <cstring>
#include <chrono>
#include <bitset>
#include <tuple>
#include <stdio.h>
#include <thread> //TODO waiting for you @wangying2023-07-13 21:56:37
#include <climits>
#include "channal_map.h"
#ifdef TO_ROOT
#include "TTree.h"
#include "TFile.h"
#include "TBranch.h"
#include "TH1F.h"
#include "TH1I.h"
#include "TGraph.h"
#include "TFitResult.h"
#include "TFolder.h"
#include "event.hpp"
#endif




template <class _tp>
requires (std::is_unsigned_v<_tp>)
inline _tp swap(_tp const& _p0){
  constexpr static std::size_t hl = sizeof(_tp)*__CHAR_BIT__/2;
  _tp rt(_p0>>hl); rt |= _p0<<hl; return rt; }


template <std::size_t _BS=8UL> //TODO
requires (_BS==4UL || _BS==8UL)
constexpr std::size_t round_up(std::size_t _bytes){
  return ((_bytes)+_BS-1) & ~(_BS-1); }
static constexpr unsigned char block_s_aa = 4;
static std::map<unsigned char,char> hex_table =
  {
    {0b00000000,'0'}
    ,{0b00000001,'1'}
    ,{0b00000010,'2'}
    ,{0b00000011,'3'}
    ,{0b00000100,'4'}
    ,{0b00000101,'5'}
    ,{0b00000110,'6'}
    ,{0b00000111,'7'}
    ,{0b00001000,'8'}
    ,{0b00001001,'9'}
    ,{0b00001010,'A'}
    ,{0b00001011,'B'}
    ,{0b00001100,'C'}
    ,{0b00001101,'D'}
    ,{0b00001110,'E'}
    ,{0b00001111,'F'}
  };
template <std::size_t NN> 
//std::ostream& debug_aa(void* _aptr, std::ostream& os=std::cout){
std::basic_string<char> debug_aa(void* _aptr){
  constexpr static std::size_t ru = round_up<8>(NN)/8;
  char const* _aptr_ref = const_cast<char const*>(reinterpret_cast<char*>(_aptr));
  std::basic_string<char> rt(2*ru,'-');
  for (auto i=0UL; i<ru; ++i){
    char const opt_aa = *_aptr_ref++;
    unsigned char comp_a = opt_aa>>4 & 0x0F;
    unsigned char comp_b = opt_aa    & 0x0F;
    rt[2*i] = hex_table[comp_a]; rt[2*i+1] = hex_table[comp_b]; }
  return rt;
}

auto const& callable_aa = [](std::basic_string<char> _p0, std::ostream& _os=std::cout)
  ->std::ostream&{
  _os<<"|"; for(auto&& x : _p0) _os<<x; return _os<< "|";
};
static auto callable_ac = []<class _tp>
//requires()
(_tp& tup, std::ostream& _os=std::cout){
  constexpr std::size_t NN = std::tuple_size_v<_tp>;
  [&]<std::size_t... I>(std::index_sequence<I...>){
    std::ostream* os_ptr = std::addressof(_os);
    (...,(*os_ptr<<"| "<<std::get<I>(tup).to_string()<<" |"));
  }(std::make_index_sequence<NN>()); };

template <class _tp,template <std::size_t,class...> class bits_view
  ,class _valid_t = bool, class... _args>
requires (std::is_same_v<_tp,unsigned char>
  || std::is_same_v<_tp, char>)  //TODO
struct event_head_t{
  constexpr static double freq = 1.2E+8; //Hz
  constexpr static std::size_t sizeof_unit = sizeof(_tp)*__CHAR_BIT__;
  typedef bits_view<sizeof_unit> bits_view_t;
  typedef event_head_t self_t;
  typedef _tp unit_type;

  template <std::size_t... _vals_v>
  using bits_block_t = std::tuple<bits_view<_vals_v>...>;

  template <class... _args_>
  using fill_t = _valid_t(*)(void*,_args_...);


  //typedef std::result_of_t<fill_t<_args...>> result_type;

  bits_view_t pack_head_tag;
  bits_block_t<1,2,13> soe_eoe_packsize;
  bits_block_t<2,6>    FEid;
  std::array<bits_view_t,6> time_stamp;
  std::array<bits_view_t,4> event_count;
  bits_block_t<2,6> hit_channal_count;
  bits_view_t reserved;
  std::array<bits_view_t,4> crc32;

  bool valid() const{ //TODO crc32_todo 
    return (pack_head_tag.to_ulong() == 0x5A)
      and (std::get<1>(soe_eoe_packsize).to_string()=="10") 
      and (std::get<2>(soe_eoe_packsize).to_ulong()==20UL);
  }

  std::string event_count_str() const{
    std::string str = "";
    for (auto&& x : event_count) str+=x.to_string();
    return str; }
  

  static std::function<_valid_t(void const*,_args...)> m_fill_rt;

  static void set_fill_rt(decltype(m_fill_rt) const& _p0){ m_fill_rt = _p0;}


  fill_t<_args...> m_fill;
  
  template <class... _args_>
  void set_fill(fill_t<_args_...> _p0) {m_fill = _p0;}

  template <class... _args_>
  decltype(auto) fill(void const* _a_add, _args_&&... _params){
    if (!_a_add) throw std::invalid_argument("...");
    if constexpr (std::is_same_v<void,_valid_t>)
      std::invoke(m_fill_rt,_a_add,std::forward<_args_...>(_params)...);
    else
      return std::invoke(m_fill_rt,_a_add,std::forward<_args_>(_params)...); }

  std::ostream& display(std::ostream& _os = std::cout) const{
    _os<<"\n"<<std::flush;
    _os<<"== pack_head_tag: "<<pack_head_tag.to_string()<<"\n";
    _os<<"== soe_eoe_packsize: "; callable_ac(soe_eoe_packsize,_os); _os<<std::endl;
    _os<<"== FEid: "; callable_ac(FEid,_os); _os<<std::endl;
    _os<<"== time_stamp: "; for (auto&& x : time_stamp) _os<<x.to_string()<<" ";
      _os<<'\n';
    _os<<"== event_count: "; for (auto&& x : event_count) _os<<x.to_string()<<" ";
      _os<<'\n';
    _os<<"== hit_channal_count: "; callable_ac(hit_channal_count,_os); _os<<"\n";
    _os<<"reserved: "<<reserved.to_string()<<"\n";
    _os<<"crc32: "; for(auto&& x : crc32) _os<<x.to_string()<<" ";
      _os<<'\n';
    _os<<"\n"<<std::flush;
    return _os; }
};



template <class _tp,template <std::size_t,class...> class bits_view
  ,class _valid_t, class... _args>
requires (std::is_same_v<_tp,unsigned char>
  || std::is_same_v<_tp, char>)
std::function<_valid_t(void const*,_args...)>
  event_head_t<_tp,bits_view,_valid_t,_args...>::m_fill_rt = std::function<_valid_t(void const*,_args...)>{};





template <class _tp
  ,template <std::size_t, class...> class bits_view
  ,class _rt_t, class... _args>
struct event_body_t{
  constexpr static std::size_t sizeof_unit = sizeof(_tp)*__CHAR_BIT__;
  typedef bits_view<sizeof_unit> bits_view_t;
  template <std::size_t... _vals_v>
  using bits_block_t = std::tuple<bits_view<_vals_v>...>;
  typedef unsigned short adc_t;
  typedef _tp unit_type;
  typedef _rt_t result_type;

  bits_view_t channal_head;
  bits_block_t<1,2,13> soe_eoe_packsize;
  bits_block_t<2,6> FEid;
  bits_block_t<1,7> channal_id;
  bits_view_t rsv_data_head;
  std::vector<adc_t> adc_dataset;
  std::array<bits_view_t,2> rsv_data_tail;
  std::array<bits_view_t,4> crc32;

  event_body_t(){ }

  unsigned int id() const{
    unsigned int rt;
    //unsigned char* aplace = reinterpret_cast<unsigned char*>(std::addressof)
    

   
  }

  bool head_valid() const{ //TODO
    return (channal_head.to_ulong()==0x5A)
      and (std::get<1>(soe_eoe_packsize).to_string()=="00");
  }
  bool data_valid() const{
    return false;
  }
  bool tail_valid() const{
    return false;
  }

  inline std::size_t pack_size() const{ return std::get<2>(soe_eoe_packsize).to_ulong(); }


  static std::function<_rt_t(void const*,_args...)> m_fill;
  static void set_fill(decltype(m_fill) const& _p0) {m_fill = _p0;}

  template <class... _args_>
  static decltype(auto) fill(void const* _aptr, _args_... _params){
    if (!_aptr) throw std::invalid_argument("...");
    if constexpr (std::is_same_v<void,_rt_t>)
      std::invoke(m_fill,_aptr,std::forward<_args_...>(_params)...);
    else 
      return std::invoke(m_fill,_aptr,std::forward<_args_>(_params)...); }

  std::ostream& display(std::ostream& _os= std::cout) const{
    _os<<"==== eb_head_tag: "<<channal_head.to_string()<<'\n';
    _os<<"==== soe_eoe_packsize: "; callable_ac(soe_eoe_packsize); _os.put('\n');
    _os<<"==== FEid: "; callable_ac(FEid); _os.put('\n');
    _os<<"==== channal_id: "; callable_ac(channal_id); _os.put('\n');
    _os<<"==== rsv_data_head: "<<rsv_data_head.to_string()<<'\n';
    //for (auto&& x : adc_dataset) _os<<"------>"<<std::bitset<16>(x).to_string()<<"\n";
    _os<<"========================: "<<adc_dataset.size()<<std::endl;
    _os<<"==== rsv_data_tail: "<<rsv_data_tail[0].to_string()<<" "<<rsv_data_tail[1].to_string()<<'\n';
    _os<<"==== crc32: "; for(auto&& x : crc32) _os<<x.to_string()<<" ";
    return _os<<'\n'<<std::flush;
  }


};
template <class _tp
  ,template <std::size_t, class...> class bits_view
  ,class _rt_t, class... _args>
std::function<_rt_t(void const*,_args...)> 
  event_body_t<_tp,bits_view,_rt_t,_args...>::m_fill = std::function<_rt_t(void const*,_args...)>{};

template <class _tp
  ,template <std::size_t, class...> class bits_view
  ,class _rt_t, class... _args>
struct event_tail_t{
  constexpr static std::size_t sizeof_unit = sizeof(_tp)*__CHAR_BIT__;
  typedef bits_view<sizeof_unit> bits_view_t;
  typedef event_tail_t self_t;
  typedef _tp unit_type;
  template <std::size_t... _vals_v>
  using bits_block_t = std::tuple<bits_view<_vals_v>...>;

  bits_view_t channal_head;
  bits_block_t<1,2,13> soe_eoe_packsize;
  bits_block_t<2,6> FEid;
  std::array<bits_view_t,4> event_size;
  std::array<bits_view_t,4> crc32;

  bool valid() const{ //TODO crc32
    return (std::get<1>(soe_eoe_packsize).to_string()=="01")
      and (std::get<2>(soe_eoe_packsize).to_ulong()==12UL);
  }

  static std::function<_rt_t(void const*,_args...)> m_fill;

  //void set_fill(std::function<_rt_t_(void*,_args_...)> _p0) {m_fill = _p0;}
  static void set_fill(decltype(m_fill) const& _p0) {m_fill = _p0;}

  template <class... _args_>
  decltype(auto) fill(void const* _aptr, _args_... _params){
    if (!_aptr) throw std::invalid_argument("...");
    if constexpr (std::is_same_v<void,_rt_t>)
      std::invoke(m_fill,_aptr,std::forward<_args_...>(_params)...);
    else 
      return std::invoke(m_fill,_aptr,std::forward<_args_>(_params)...); }

  std::ostream& display(std::ostream& _os = std::cout) const{
    _os.put('\n');
    _os<<"== pack_tail_tag: "<<channal_head.to_string()<<std::endl;
    _os<<"== soe_eoe_packsize: "; callable_ac(soe_eoe_packsize,_os);_os.put('\n');
    _os<<"== FEid: "; callable_ac(FEid,_os); _os.put('\n');
    _os<<"== event_size: "; for(auto&& x : event_size) _os<<x.to_string()<<" ";
    _os<<"\n== crc32: "; for(auto&& x : crc32) _os<<x.to_string()<<" ";
    return _os<<'\n'<<std::flush;
  }

};

template <class _tp
  ,template <std::size_t, class...> class bits_view
  ,class _rt_t, class... _args>
std::function<_rt_t(void const*,_args...)> event_tail_t<_tp,bits_view,_rt_t,_args...>::m_fill
  = std::function<_rt_t(void const*, _args...)>{};


template <class _ehead_t, class _ebody_t, class _etail_t
  ,template <class,class...> class _cont_tt = std::vector, std::size_t bufsize=256> //TODO
requires (bufsize >= 256)
struct file_unpacker{
  typedef _ehead_t eh_t;
  typedef _ebody_t eb_t;
  typedef typename eb_t::result_type result_type;
  typedef _cont_tt<_ebody_t> _ebody_tt;
  typedef _etail_t et_t;
  typedef typename _ehead_t::unit_type unit_type;
private:
  enum class stream_status_t : unsigned char {ES=0,EB,EE,UNKNOW};
  enum class event_status_t : unsigned char {COMP=0,INCOMP,UNKNOW};
public:
  eb_t* aplace;


  unit_type buffer[bufsize];

  file_unpacker(){ stream_status = stream_status_t::UNKNOW; }

  //template <class... _args>
  //result_type fill_eb(void* _aptr, _args&&... _params){
  //  if constexpr (std::is_same_v<result_type,void>)
  //    eb_t::fill(_aptr,std::forward<_args>(_params)...);
  //  else return eb_t::m_fill(_aptr,std::forward<_args>(_params)...);
  //}
  
  
  std::filesystem::path in_file;
  void setfile(char const* _ptr){ in_file = std::filesystem::path(_ptr); }
  void setfile(std::filesystem::path const& _p0) {in_file = _p0;}
  FILE* m_fptr;

  _ehead_t Event_head;
  _cont_tt<_ebody_t> Event_body;
  _etail_t Event_tail;
  stream_status_t stream_status = stream_status_t::UNKNOW;
  event_status_t  event_status  = event_status_t::UNKNOW;

  struct event_pack{
    typedef event_pack self_t;
    _ehead_t m_eh = _ehead_t{};
    _cont_tt<_ebody_t> m_eb = _cont_tt<_ebody_t>{};
    _etail_t m_et = _etail_t{};
    event_pack() = default;
    ~event_pack() = default;
    event_pack(_ehead_t const& _p0, _cont_tt<_ebody_t> const& _p1, _etail_t const& _p2)
      :m_eh(_p0) ,m_eb(_p1) ,m_et(_p2) {}

    self_t& set_evt_h(_ehead_t const& _p0) {m_eh = _p0; return *this;}
    template <class _iter_t>
    //requires () //TODO
    self_t& set_evt_b(_iter_t const& _p0, _iter_t const& _p1){
      m_eb.resize(std::distance(_p0,_p1));
      std::copy(_p0,_p1,std::begin(m_eb));
      return *this;
    };

    //template <t>
    self_t& set_evt_t(_etail_t const& _p0) {m_et = _p0; return *this;}
    std::ostream& display(std::ostream& _os = std::cout){
      m_eh.display();
      _os<<"------: _cont_tt<_ebody_ttn>: "<<m_eb.size()<<std::endl;
      m_et.display();
      return _os;
    }
  };
  typedef event_pack evt_pack_t;

  //event_pack make_evtpack
  
  template <template <class,class...>class _cont_tt_=std::list>
  bool parse(_cont_tt_<event_pack>& _dest, std::size_t evt_ct=(std::numeric_limits<std::size_t>::max)()){
    aplace = new eb_t{};
    m_fptr = fopen(in_file.string().c_str(),"r");
    fseek(m_fptr,0,SEEK_SET);
    auto const& fill_buffer __attribute__((__unused__))= [&,this](FILE* _p0){
      std::fill_n(buffer,0,bufsize);
      std::size_t _index = 0;
      while(_index<bufsize && !feof(_p0))
        fread(&buffer[_index++],sizeof(unit_type),1,m_fptr); };
    auto const& fill_buffer_rt1 = 
      [&,this]<class _tp>(FILE* _p0,_tp* _aptr, std::size_t N){
      fread(_aptr,sizeof(_tp),N,m_fptr); 
      //for (auto i=0UL; i<N; ++i)
      //  fread(_aptr++,sizeof(unit_type),1,m_fptr);
    };
    auto* a_add = new unit_type;
    std::size_t eb_count = 0;

    auto const& is_tail_aa = [](void* _aptr)->bool{
      if (!_aptr) return false;
      unsigned char* _aptr_ref = static_cast<unsigned char*>(_aptr);
      return _aptr_ref[0]==0x5A and _aptr_ref[1]==0x20 and _aptr_ref[2]==0x0C;
    };

    std::size_t index_=0;
    while(!feof(m_fptr)){
      fill_buffer_rt1(m_fptr,a_add,1);
      index_++;
      if(index_%4096==0) info_out(index_/4096);
      if (*a_add == 0x5A and stream_status == stream_status_t::UNKNOW){
        fseek(m_fptr,-1,SEEK_CUR);
        unit_type a_add_aa[20];
        fill_buffer_rt1(m_fptr,a_add_aa,20);
        Event_head.fill(a_add_aa,20);
        if (Event_head.valid()){
          stream_status = stream_status_t::ES;
          continue;
        }
        fseek(m_fptr,-19,SEEK_CUR);
        continue;
      }
      if (*a_add == 0x5A and 
          (stream_status == stream_status_t::ES or stream_status == stream_status_t::EB)
          and eb_count<96
          ){
        fseek(m_fptr,-1,SEEK_CUR);
        unit_type a_add_aa[2060];
        fill_buffer_rt1(m_fptr,a_add_aa,2060);
        if (is_tail_aa((void*)a_add_aa)){
          fseek(m_fptr,-2060,SEEK_CUR);
          stream_status = stream_status_t::EE;
          continue;
        }
        eb_t::fill(a_add_aa);
        if (aplace->head_valid()){
          if (aplace->pack_size() != 2060){
            fseek(m_fptr,-2060,SEEK_CUR);
            fseek(m_fptr,aplace->pack_size(),SEEK_CUR);
            //TODO
            eb_count++;
            stream_status = stream_status_t::EB;
            continue;
          }
          Event_body.emplace_back(*aplace);
          stream_status = stream_status_t::EB;
          eb_count++;
        }
        continue;
      }
      if (*a_add == 0x5A and 
          (stream_status == stream_status_t::EB or stream_status==stream_status_t::EE)
          and eb_count<=96){
        fseek(m_fptr,-1,SEEK_CUR);
        unit_type a_add_aa[12];
        fill_buffer_rt1(m_fptr,a_add_aa,12);
        Event_tail.fill(a_add_aa);
        if (Event_tail.valid())
          event_status = event_status_t::COMP, 
          stream_status = stream_status_t::UNKNOW, eb_count=0, evt_ct-- ;
        else
          throw std::runtime_error("..."); //TODO
        if (evt_ct==0) break;
      }
      if (__builtin_expect(event_status==event_status_t::COMP,0)){
        _dest.push_back(event_pack()
            .set_evt_h(Event_head)
            .set_evt_b(std::begin(Event_body),std::end(Event_body))
            .set_evt_t(Event_tail));
        event_status = event_status_t::UNKNOW;
        if (_dest.size()==1000){
          //for(auto&& x : _dest)
            //info_out(x.m_eh.)
          _dest.clear(), info_out("TAG");
        }
        Event_body.clear();
      }
    }
    fclose(m_fptr);
    delete a_add; a_add = nullptr; delete aplace; aplace = nullptr;
    return true;
  }

  std::ostream& display(std::ostream& _os=std::cout) const{
    Event_head.display(_os);
    for (auto&& x : Event_body) x.display(_os);
    Event_tail.display(_os);
    return _os;
  }
};

using vlast_beamfile_unpacker = file_unpacker<
  event_head_t<unsigned char,std::bitset,void,std::size_t>
  ,event_body_t<unsigned char,std::bitset,void>
  ,event_tail_t<unsigned char,std::bitset,void>>;
 


int main(int argc, char* argv[]){
  //char const* filename = "./work_dat2conv/20230618_200fC.dat";
  //char const* filename = "./work_dat2conv/20230709_HV382_BGO_22_50.dat";
  //char const* filename = "20230710_HV382_BGO_13_06.dat";
  char const* filename = "20230711_HV382_BGO_09_46.dat";
  //char const* filename = "202204151834.dat";
  std::ifstream fin;
  fin.open(filename);
  if (!fin) throw std::invalid_argument("...");
  fin.seekg(0,std::ios::beg);
  std::istream_iterator<char> begin(fin);
  std::istream_iterator<char> end{};
  //for (int i=1; i<=2200; ++i){
  //  //callable_aa(debug_aa<8>((void*)std::addressof(*begin++)));
  //  std::cout<<"|"<<std::bitset<8>(*begin++).to_string()<<"|";
  //  if (__builtin_expect(i%8==0 and i!=0,0)) std::cout<<"\n"<<std::flush;
  //}
  std::cout<<std::endl;

  //std::size_t index = 0;
  fin.close();
  //std::array<unsigned char,20> a_test = {
  //  0x5A, 0x40,0x14 ,0x11 ,0x00 ,0x01 ,0x11 ,0xF1 ,0x6A  ,0x88 
  //   ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x14 ,0x9C ,0xC2 ,0x57 };

  //std::array<unsigned char,>
  event_head_t<unsigned char,std::bitset,void,std::size_t> event_head;
  auto callable_ab = [&event_head](void const* _aptr,std::size_t _size)->void{
    //auto const* _ptr = const_cast<unsigned char const*>
    //  (reinterpret_cast<unsigned char*>(_aptr));
    auto const* _ptr = static_cast<unsigned char const*>(_aptr);
    event_head.pack_head_tag = std::bitset<8>(*_ptr++);
    std::get<2>(event_head.soe_eoe_packsize) = std::bitset<13>(
        swap(*reinterpret_cast<unsigned short*>(const_cast<unsigned char*>(_ptr))));
    _ptr+=2;
    std::get<0>(event_head.FEid) = std::bitset<2>(*_ptr>>6);
    std::get<1>(event_head.FEid) = std::bitset<6>(*_ptr++);
    for(int i=5; i>=0; --i) event_head.time_stamp[i] = std::bitset<8>(*_ptr++);
    for(int i=3; i>=0; --i) event_head.event_count[i] = std::bitset<8>(*_ptr++);
    std::get<0>(event_head.hit_channal_count) = std::bitset<2>(*_ptr>>6);
    std::get<1>(event_head.hit_channal_count) = std::bitset<6>(*_ptr++);
    event_head.reserved = std::bitset<8>(*_ptr++);
    for (int i=3; i>=0; --i) event_head.crc32[i] = std::bitset<8>(*_ptr++);
  };




  event_head.set_fill_rt(callable_ab);
  //void* _bgn = reinterpret_cast<void*>(std::addressof(*a_test.begin()));
  //callable_ab(_bgn,20);
  //event_head.fill(_bgn,20);
  //event_head.display();

  vlast_beamfile_unpacker opt_aa;
  opt_aa.setfile(filename);


  auto callable_ab_rt0 = [&opt_aa](void const* _aptr,std::size_t _size)->void{
    auto const* _ptr = static_cast<unsigned char const*>(_aptr);
    opt_aa.Event_head.pack_head_tag = std::bitset<8>(*_ptr++);
    std::get<0>(opt_aa.Event_head.soe_eoe_packsize) = std::bitset<1>(*_ptr>>7);
    std::get<1>(opt_aa.Event_head.soe_eoe_packsize) = std::bitset<2>(*_ptr>>5);
    std::get<2>(opt_aa.Event_head.soe_eoe_packsize) = std::bitset<13>(
        swap(*reinterpret_cast<unsigned short*>(const_cast<unsigned char*>(_ptr))));
    _ptr+=2;
    std::get<0>(opt_aa.Event_head.FEid) = std::bitset<2>(*_ptr>>6);
    std::get<1>(opt_aa.Event_head.FEid) = std::bitset<6>(*_ptr++);
    for(int i=5; i>=0; --i) opt_aa.Event_head.time_stamp[i] = std::bitset<8>(*_ptr++);
    for(int i=3; i>=0; --i) opt_aa.Event_head.event_count[i] = std::bitset<8>(*_ptr++);
    std::get<0>(opt_aa.Event_head.hit_channal_count) = std::bitset<2>(*_ptr>>6);
    std::get<1>(opt_aa.Event_head.hit_channal_count) = std::bitset<6>(*_ptr++);
    opt_aa.Event_head.reserved = std::bitset<8>(*_ptr++);
    for (int i=3; i>=0; --i) opt_aa.Event_head.crc32[i] = std::bitset<8>(*_ptr++); };

  auto callable_ae = [&opt_aa](void const* _aptr)->void{
    typedef typename std::remove_cvref_t<decltype(opt_aa.Event_tail)>::unit_type unit_type;
    unit_type const* _ptr = static_cast<unit_type const*>(_aptr);
    constexpr static std::size_t NN = sizeof(unit_type)*__CHAR_BIT__;
    opt_aa.Event_tail.channal_head = std::bitset<NN>(*_ptr++);
    std::get<0>(opt_aa.Event_tail.soe_eoe_packsize) = std::bitset<1>(*_ptr>>7);
    std::get<1>(opt_aa.Event_tail.soe_eoe_packsize) = std::bitset<2>(*_ptr>>5);
    std::get<2>(opt_aa.Event_tail.soe_eoe_packsize) = std::bitset<13>(
        swap(*reinterpret_cast<unsigned short*>(const_cast<unsigned char*>(_ptr))));
    _ptr += 2;
    std::get<0>(opt_aa.Event_tail.FEid) = std::bitset<2>(*_ptr>>6);
    std::get<1>(opt_aa.Event_tail.FEid) = std::bitset<6>(*_ptr++);
    //for (int i=0; i<8; ++i) info_out(std::bitset<8>(*_ptr++));
    //std::vector<std::bitset<8>> array_aa;
    //for (int i=0; i<8; ++i) array_aa.emplace_back(std::bitset<8>(*_ptr++));
    //std::copy(array_aa.begin(),array_aa.begin()+4,opt_aa.Event_tail.event_size.begin());
    //std::copy(array_aa.begin()+4,array_aa.begin()+8,opt_aa.Event_tail.crc32.begin());
    for (int i=0; i<4; ++i) opt_aa.Event_tail.event_size[i] = std::bitset<8>(*_ptr++);
    for (int i=0; i<4; ++i) opt_aa.Event_tail.crc32[i] = std::bitset<8>(*_ptr++);
  };

  std::size_t _size = 1024;
  auto callable_af = [&opt_aa,&_size](void const* _aptr)->void{
    //typedef typename std::remove_cvref_t<decltype(opt_aa.Event_body)>::unit_type unit_type;
    typedef typename decltype(opt_aa)::eb_t eb_t;
    auto* aplace = opt_aa.aplace;
    auto const& fill_ebh = [&](void const* _aptr){
      unsigned char const* _ptr = static_cast<unsigned char const*>(_aptr);
      aplace->channal_head = std::bitset<8>(*_ptr++);
      std::get<0>(aplace->soe_eoe_packsize) = std::bitset<1>(*_ptr>>7);
      std::get<1>(aplace->soe_eoe_packsize) = std::bitset<2>(*_ptr>>5);
      std::get<2>(aplace->soe_eoe_packsize) = std::bitset<13>(
        swap(*reinterpret_cast<unsigned short*>(const_cast<unsigned char*>(_ptr))));
      _ptr+=2;
      std::get<0>(aplace->FEid) = std::bitset<2>(*_ptr>>6);
      std::get<1>(aplace->FEid) = std::bitset<6>(*_ptr++);
      std::get<0>(aplace->channal_id) = std::bitset<1>(*_ptr>>7);
      std::get<1>(aplace->channal_id) = std::bitset<7>(*_ptr++);
      aplace->rsv_data_head = std::bitset<8>(*_ptr);
    };
    aplace->adc_dataset.clear();
    auto const& fill_ebadc = [&](void const* _ptr){
      typedef typename eb_t::adc_t adc_t;
      adc_t const* adr_aa = static_cast<adc_t const*>(_ptr);
      for (auto i=0UL; i<_size; ++i) 
        aplace->adc_dataset.emplace_back(swap(*adr_aa++));
    };

    auto const& fill_ebt = [&](void const* _aptr){
      unsigned char const* _ptr = static_cast<unsigned char const*>(_aptr);
      aplace->rsv_data_tail[0] = std::bitset<8>(*_ptr++);
      aplace->rsv_data_tail[1] = std::bitset<8>(*_ptr++);
      for (int i=3; i>=0; --i) aplace->crc32[i] = std::bitset<8>(*_ptr++);
    };
    fill_ebh(_aptr);
    fill_ebadc((void const*)((unsigned char const*)_aptr+6));
    fill_ebt((void const*)((unsigned char const*)_aptr+6+_size*2));
  };


  vlast_beamfile_unpacker::eh_t::set_fill_rt(callable_ab_rt0);
  vlast_beamfile_unpacker::eb_t::set_fill(callable_af);
  vlast_beamfile_unpacker::et_t::set_fill(callable_ae);
  //opt_aa.Event_head.set_fill_rt(callable_ab_rt0);
  //opt_aa.Event_tail.set_fill(callable_ae);
  //opt_aa.Event_body.set_fill(callable_af);

  typedef typename vlast_beamfile_unpacker::evt_pack_t evt_pack_t;
  std::vector<evt_pack_t> dest;
  opt_aa.parse<std::vector>(dest);
  for (auto&& x : dest)
    info_out(x.m_eb.size());

  /*
  for (auto&& x : dest){
    for(auto&& y : x.m_eb)
      info_out(std::get<1>(y.channal_id).to_ulong());
  }
  */
  for(auto&&x :dest)
    x.m_et.display();
  info_out(dest.size());


  //info_out(dest[0].m_eb[0].adc_dataset.size());
  //for (auto&& x : dest[0].m_eb[0].adc_dataset)
  //  std::cout<<std::hex<<x<<std::endl;

  
  
  //for (auto&& x : dest) x.display();
  //for (auto&& x : dest) x.display();
  //exit(0);


  //unsigned short xxxxx = 0xFFEE;
  //info_out(std::bitset<16>(swap(xxxxx)).to_string());
  //opt_aa.display();
  //for (int i=0; i<10; ++i)
  //for (auto i=0UL; i<dest.size(); ++i)
  //  dest[i].m_eh.display(), dest[i].m_eb[0].display(), dest[i].m_et.display();
  //for (auto&& x : dest){
  //  for (auto&& y : x.m_eh.event_count) std::cout<<y.to_string()<<" ";
  //  std::cout<<std::endl;
  //}
  std::string rawname{filename};
  std::string root_file_name(
      std::begin(rawname)+rawname.find_last_of('/')+1
      ,std::begin(rawname)+rawname.find_last_of('.'));
  root_file_name += "X__TAG__X";
  struct root_data_perfile{
    unsigned short ADM_id;
    double time_stamp;
    unsigned int evt_id;
    unsigned short channal_hits;
  };
  struct root_data_perevt{
    std::vector<unsigned int> channal_ID;
    std::vector<std::array<unsigned short,1024>> adc;
  };
  //info_out(root_file_name);
  auto const& callable_ai = 
    []<std::size_t I, std::size_t J>
    (std::bitset<I>& _p0, std::bitset<J> const& _form, std::size_t _size){
    for (auto i = 0UL; i<_form.size();++i)
      _p0.set(i+_size,_form[i]); };
  root_data_perfile aplace_aa;
  root_data_perevt  aplace_ab;

  typedef typename vlast_beamfile_unpacker::eh_t eh_t;
  typedef typename vlast_beamfile_unpacker::_ebody_tt eb_t;
  typedef typename vlast_beamfile_unpacker::et_t et_t;
  auto const& callable_ag = [&aplace_aa](eh_t const& _p0)->void{
    constexpr static double fz = eh_t::freq;
    std::string abuf{};
    for(auto iter=_p0.time_stamp.rbegin(); iter != _p0.time_stamp.rend(); ++iter)
      abuf+=iter->to_string();
    std::bitset<64> bits_aa(abuf);
    abuf.clear();
    for (auto iter=_p0.event_count.rbegin(); iter != _p0.event_count.rend(); ++iter)
      abuf += iter->to_string();
    std::bitset<32> bits_ab(abuf);
    aplace_aa.ADM_id = (unsigned short)(std::get<1>(_p0.FEid).to_ulong());
    aplace_aa.time_stamp = bits_aa.to_ulong()/fz;
    aplace_aa.evt_id = (unsigned int)bits_ab.to_ulong();
    aplace_aa.channal_hits = (unsigned short)(std::get<1>(_p0.hit_channal_count).to_ulong());
  };
  auto const& callable_ah = [&aplace_ab](eb_t const _p0)->void{
    aplace_ab.channal_ID.clear();
    aplace_ab.adc.clear();
    for (auto&& x : _p0){
      std::array<unsigned short,1024> abuf;
      aplace_ab.channal_ID.emplace_back(std::get<1>(x.channal_id).to_ulong());
      std::size_t index_=0;
      for(auto iter=x.adc_dataset.begin(); iter != x.adc_dataset.end(); ++iter)
        abuf[index_++] = (*iter)&0x0FFF;
      aplace_ab.adc.emplace_back(abuf);
    }
  };
  /*
  std::vector<unsigned short> max_adc;
  for (auto&& x : dest){
    callable_ah(x.m_eb);
    std::vector<unsigned short> abuf;
    for (auto&& y : aplace_ab.adc){
      abuf.push_back(*std::max_element(std::begin(y),std::end(y)));
      std::cout<<*std::max_element(std::begin(y),std::end(y));
      //for (auto&& z : y)
      //  std::cout<<z<<std::endl;
    }
    //std::cout<<"-------------------"<<std::endl;
    max_adc.push_back(*std::max_element(std::begin(abuf),std::end(abuf)));
  }
  info_out(max_adc.size());
  std::sort(std::begin(max_adc),std::end(max_adc));
  for (auto&& x : max_adc) info_out(x);
  */




  
#ifdef TO_ROOT
  auto* rootf_out = new TFile((root_file_name+".root").c_str(),"RECREATE");
  auto root_view = [&](std::size_t _p0)->void{
    callable_ah(dest[_p0].m_eb);
    std::string evt_tag;
    for (auto iter = std::rbegin(dest[_p0].m_eh.event_count); iter != rend(dest[_p0].m_eh.event_count); ++iter)
      evt_tag += iter->to_string();
    std::string evt_name = "evt: "+std::to_string(std::bitset<32>(evt_tag.c_str()).to_ulong());
    TFolder* afld = new TFolder(evt_name.c_str(),evt_name.c_str());

    float time_bin[1024]; std::iota(std::begin(time_bin),std::end(time_bin),0.f);
    std::vector<TGraph*> abuf_;
    for(auto i=0UL; i<aplace_ab.channal_ID.size(); ++i){
      if (channal_table.find(aplace_ab.channal_ID.at(i)) == channal_table.end())
        throw std::invalid_argument("...");
      float abuf[1024];
      std::transform(std::begin(aplace_ab.adc[i]),std::end(aplace_ab.adc[i]),std::begin(abuf)
          ,[](unsigned short _p0){return 4096.f-(float)(_p0);});
      std::string g_tag = "graph_"+channal_table[aplace_ab.channal_ID[i]];
      auto* agrh = new TGraph(1024,time_bin,abuf);
      agrh->SetName(g_tag.c_str());
      afld->Add(agrh);
      abuf_.push_back(agrh);
      //delete agrh; agrh=nullptr;
    }
    afld->Write();
    //for(auto&& x : abuf_) delete x; abuf_.clear();
    //delete afld; afld=nullptr;
  };
  root_view(70);
  root_view(200);
  root_view(300);
  root_view(400);
  root_view(500);

  TTree* atree = new TTree("atree","atree");
  double aplace_ac;
  VL_CRtest_BgoRaw* aplace_ad = new VL_CRtest_BgoRaw{};
  aplace_ad->evt_time = 3.14;
  aplace_ad->id = 1234;
  for (int i=0; i<10; ++i) aplace_ad->channels_id.push_back(i);
  aplace_ad->channels_adc.resize(10);
  for (int i=0; i<10; ++i){
    std::array<unsigned short,1024> aplace_ae; 
    std::iota(std::begin(aplace_ae),std::end(aplace_ae),111+i);
    std::copy(std::begin(aplace_ae),std::end(aplace_ae)
        ,std::begin(aplace_ad->channels_adc[i].m_adc));
  }
  auto callable_aj = [&aplace_ad](std::ostream& _os = std::cout)->std::ostream&{
    _os<<"evt_time: "<<aplace_ad->evt_time<<" id: "<<aplace_ad->id<<std::endl;
    for (auto&& x : aplace_ad->channels_id) _os<<x<<" ";
    _os<<std::endl;
    for (auto&& x : aplace_ad->channels_adc)
      _os<<x.m_adc[0]<<" "<<x.m_adc[1]<<" "<<x.m_adc[2]<<std::endl;
    return _os;
  };

  std::string nama_aa = rootf_out->GetName();
  atree->Branch("CR_BgoRaw",aplace_ad);
  atree->Fill();
  atree->Fill();
  atree->Fill();
  atree->Write();
  rootf_out->Write();
  rootf_out->Close();


  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  info_out(".........................");
  auto* aplace_af = new TFile(nama_aa.c_str());
  //info_out(aplace_af); exit(0);
  auto* atree_aa = static_cast<TTree*>(aplace_af->Get("atree"));
  atree_aa->SetBranchAddress("CR_BgoRaw",std::addressof(aplace_ad));
  //info_out(atree_aa->GetEntries()); exit(0);
  atree->GetEntry(1); //OK
  callable_aj();

#endif


  


  return 0;
}
