//--------------------------------Stamp-------------------------------
//^-^ Author: Zhi Heng            Email: wy187110@mail.ustc.edu.cn     
//^-^ Time: 2024-03-18 16:10:31   Posi: Hefei
//^-^ File: /home/wangying/read_new.cxx
//--------------------------------------------------------------------
#define info_out(X) std::cout<<"==> "<<__LINE__<<" "<<#X<<" |"<<(X)<<"|\n"
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <fcntl.h>
#include <cstdint>
#include <bitset>
#include <map>
#include <sstream>

#include "TFolder.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1F.h"

template <class _tp>
struct id{
  std::string name;
  _tp range;

  virtual ~id() noexcept = default;
};

template <class _tp, class member_t, class rt_t, class... args>
struct data_region : public id<std::array<uint64_t,2>>, public member_t{
  typedef _tp unit_type;
  typedef id id_t;
  typedef member_t member_type;

public:
  data_region() = default;
  virtual ~data_region() noexcept = default;

  virtual std::ostream& display(std::ostream& os = std::cout) const{
    return os; }

  virtual rt_t fill(args...) = 0;
  virtual bool valid() const = 0;
  virtual uint16_t size() const = 0;
  virtual bool crc32() const = 0;
};

struct event_head{
  typedef unsigned char unit_type;
  unit_type data[20]; };

struct event_body{
  constexpr static size_t adc_count = 512;
  typedef unsigned char unit_type;
  unit_type data[12];
  unit_type* adc;

  explicit event_body(){ adc = new unit_type[adc_count*2]; }

  ~event_body() noexcept{ delete[] adc; }
};


struct event_tail{
  typedef unsigned char unit_type;
  unit_type data[12];
};


struct data_head : 
  public data_region<unsigned char, event_head,size_t,FILE*>{
  constexpr static double cs_fz = 1.2E+8;

  typedef data_region base_t;
  typedef typename event_head::unit_type unit_type;
  //using base_t::data;

  data_head() = default;
  data_head(FILE* fp){
    fill(fp);
  }
  virtual ~data_head() noexcept = default;


  virtual size_t fill(FILE* fptr) override{
    this->data[0] = 0x5a;
    return fread(this->data+1,sizeof(unit_type),19,fptr); };

  //TODO CRC32
  virtual bool valid() const override{
    unsigned char const* dp = this->data;
    return dp[0]==0x5A && 
      std::bitset<8>(dp[1]).to_string().substr(1,2) == "10"
      && size()==20;
  }

  virtual uint16_t size() const override{
    unsigned char const* dp = this->data;
    uint16_t result = (dp[1]<<8) + dp[2];

    //std::cout<<std::bitset<16>(result & 0x1FFF).to_string()<<std::endl;
    return result & 0x1FFF;
  };

  virtual bool crc32() const override{
    std::cout<<"TODO!!!"<<std::endl;
    return true;
  }

  double get_time() const{
    unsigned char const* dp = this->data;
    uint64_t time_uint{};
    for (int i=5; i>=0; ++i) time_uint |= (dp[9-i]<<i*8);
    return time_uint/cs_fz; }
  uint32_t get_event_id() const{
    uint32_t rt{};
    rt |= this->data[10]<<24; rt |= this->data[11]<<16;
    rt |= this->data[12]<<8; rt |= this->data[13]<<0;
    return rt; }
  uint8_t get_channel_count() const{
    return this->data[14] & 0b01111111; }

};

struct data_body : public data_region<unsigned char, event_body,size_t,FILE*>{
  typedef data_region<unsigned char, event_body,size_t,FILE*> base_t;
  typedef typename base_t::unit_type unit_type;

  bool _m0=false;// Is adc data point equal to pack size;

public:
  data_body() = default;
  virtual ~data_body() noexcept = default;

  data_body(FILE* fp){
    fill(fp);
  }

  virtual size_t fill(FILE* fp) override{
    this->data[0] = 0x5A;
    auto count0 = fread(this->data+1,sizeof(unit_type),5,fp);
    size_t count1 __attribute__((unused)) = fread(this->adc,sizeof(unit_type),1024,fp);
    //_m0 = (size()-6==count1);
    auto count2 = fread(this->data+6,sizeof(unit_type),6,fp);
    return count0+count2;
  }

  virtual bool valid() const override{
    unsigned char const* dp = this->data;
    return dp[0]==0x5A &&
      std::bitset<8>(dp[1]).to_string().substr(1,2) == "00"
      && size()==1036; }

  virtual uint16_t size() const override{
    unsigned char const* dp = this->data;
    uint16_t result = (dp[1]<<8) + dp[2];
    return result & 0x1FFF;
  }

  virtual bool crc32() const override{
    std::cout<<"TODO!!!"<<std::endl;
    return true;
  };

  uint8_t FE_ID() const{ return data[3] & 0b00111111; }
  uint8_t channal_index() const {return data[4] & 0b01111111;}
};


struct data_tail : public data_region<unsigned char,event_tail,size_t,FILE*>{
  typedef data_tail self_t;
  typedef data_region<unsigned char, event_tail,size_t,FILE*> base_t;
  typedef typename event_tail::unit_type unit_type;

  data_tail() = default;
  virtual ~data_tail() noexcept = default;

  data_tail(FILE* fp){ fill(fp); }

  virtual size_t fill(FILE* fp) override{
    this->data[0] = 0x5a;
    return fread(this->data+1,sizeof(unit_type),11,fp); }

  virtual bool valid() const override{
    unsigned char const* dp = this->data;
    return dp[0]==0x5A && 
      std::bitset<8>(dp[1]).to_string().substr(1,2) == "01" && size()==12; }

  virtual uint16_t size() const override{
    unsigned char const* dp = this->data;
    uint16_t result = (dp[1]<<8) + dp[2];
    return result & 0x1FFF; }

  virtual bool crc32() const override {
    std::cout<<"TODO!!!"<<std::endl;
    return true;
  }
};



template <class _tp, class _up , class _fp>
struct unpacker{

  typedef id<std::array<uint64_t,2>> id_t;
  typedef struct{
    std::vector<_tp*> m_heads;
    std::vector<_up*> m_bodys;
    std::vector<_fp*> m_tails;

    void clear(){
      for (auto&& x : m_heads) delete x;
      for (auto&& x : m_bodys) delete x;
      for (auto&& x : m_tails) delete x; }
  } event_t;


  //std::map<uint32_t,std::vector<id_t*>>  in_memory;
  std::map<uint32_t,event_t> in_memory;
  uint32_t active_event = (std::numeric_limits<uint32_t>::max)();

  void clear(){
    for (auto&& x : in_memory) x.second.clear();
    in_memory.clear();
  }
  std::string m_fname;

  void setfile(std::string const& v) {m_fname = v;}




  void parse(){
    
    //std::string fname = "RunID66683_20240311L1549_HEIC-Cube_Sci.dat";
    auto* fp = fopen(m_fname.c_str(),"r");
    
    if (!fp){
      throw std::invalid_argument("a invalid input file name");
    }
    info_out(ftell(fp));
    //for ( int i=0; i<10; ++i){
    //  std::cout<<fseek(fp,0,SEEK_END)<<" "<<ftell(fp)<<std::endl;
    //  std::this_thread::sleep_for(std::chrono::milliseconds(300));
    //}

    while(!feof(fp)){
      unsigned char data;
      fread(&data,sizeof(unsigned char),1,fp);
      //info_out(fread(&data,sizeof(unsigned char),1,fp));
      //std::cout<<std::hex<<data<<std::dec<<std::endl;
      if (data==0x5A){

        long back = ftell(fp);
        _tp* head = new _tp(fp);
        if (head->valid()){
          uint32_t evt_id = head->get_event_id();
          //info_out(head->size());

          //std::cout<<"head get: "<<evt_id<<" "<<ftell(fp)-1<<" "<<head->size()<<std::endl;
          head->name = "head";
          uint64_t pos = (uint64_t)ftell(fp)-1;
          head->range[0] = pos; 
          head->range[1] = pos+head->size();
          //in_memory[evt_id].emplace_back(dynamic_cast<id_t*>(head));
          in_memory[evt_id].m_heads.emplace_back(head);
          active_event = evt_id;
          //fseek(fp,-1,SEEK_CUR);
          continue;
        }
        fseek(fp,back,SEEK_SET);
        delete head;

        back = ftell(fp);
        _up* body = new _up(fp);
        if (body->valid()){
          //std::cout<<"body get: "<<body->size()<<std::endl;
          body->name = "body";
          uint64_t pos = (uint64_t)ftell(fp)-1;
          head->range[0] = pos; 
          head->range[1] = pos+body->size();
          //in_memory[active_event].emplace_back(dynamic_cast<id_t*>(body));
          in_memory[active_event].m_bodys.emplace_back(body);
          continue;
        }
        fseek(fp,back,SEEK_SET);
        delete body;
        
        back = ftell(fp);
        auto* tail = new _fp(fp);
        if (tail->valid()){
          //std::cout<<"tail get: "<<ftell(fp)-1<<std::endl;
          //fseek(fp,-1,SEEK_CUR);
          tail->name = "tail";
          uint64_t pos = (uint64_t)ftell(fp)-1;
          head->range[0] = pos; 
          head->range[1] = pos+tail->size();
          //in_memory[active_event].emplace_back(dynamic_cast<id_t*>(tail));
          in_memory[active_event].m_tails.emplace_back(tail);
          continue;
        }
        fseek(fp,back,SEEK_SET);
        delete tail;
        //fseek(fp,-11,SEEK_CUR);

        //std::cout<<ftell(fp)<<std::endl;
      }
      //if (ftell(fp)>=50000) break;
    }
    fclose(fp);
       

  }

  //std::ostream& display(std::ostream& = std::cout){
  //  for (auto&& [_,__] : in_memory)
  //    std::cout<<_<<" "<<__.size()<<"\n";
  //}
  
};
//---------------------------------------------------------------------
// THIS IS JUST VERSION -1, I NEED TODO:
//  crc32 and data compression **
//  a good package and 'dynamic_cast' technology roadmap *
//  multi-thread, timet, memory pool and fragment combination *** (quick,more quick!!!)
//  GUI for client **
//    ncurses first and Qt second
//  data stream: ***
//    I need a class as: template <class _tp, class _up>, which '_tp' is prev data format 
//    such as '.dat', and '_up' is next data format such as '.root' or 'protobuf'
//
//When you should do to use this small 'framework' 
//1. your head, boay and tail data class 
//  class head: "read.cpp" Line 52
//  class body  "read.cpp" Line 56 is author test version
struct event_body_new{
  typedef unsigned char unit_type;
  unit_type data[17];
};
//  class tail "read.cpp" Line 68
//
//2. use the components in 1 as template argument for data_region (define in "temp.cpp" Line 34)
//    and inherit it
//data_region
//  template param0 [_tp] unit type
//  template param1 [member_t] your data class(ie. event_body_new)
//  template param2 [rt_t] result type for pure virtual function 'fill'
//  template param3 [args] arguments type for pure virtual function 'fill'

/* struct data_head : public data_region<unsigned char, event_head,size_t,FILE*> */ 
  // in "temp.cpp" Line 74

struct data_body_new : public data_region<unsigned char,event_body_new,size_t,FILE*>{
  typedef data_body_new self_t;
  typedef data_region base_t;
  typedef typename event_body_new::unit_type unit_type;

public:
  data_body_new() = default;
  ~data_body_new() noexcept = default;

  data_body_new(FILE* fp){
    fill(fp); }

//2.1 you should achieve all the pure virtual functions(fill,size,valid,crc32)
  virtual size_t fill(FILE* fp) override{
    this->data[0] = 0x5A;
    return fread(this->data+1,sizeof(unsigned char),16,fp);
  }
  virtual bool valid() const override{
    unsigned char const* dp = this->data;
    return dp[0]==0x5A && 
      std::bitset<8>(dp[1]).to_string().substr(1,2) == "00" && size()==17; }
  
  virtual uint16_t size() const{
    unsigned char const* dp = this->data;
    uint16_t result = (dp[1]<<8) + dp[2];
    return result & 0x1FFF; }

  virtual bool crc32() const override{
    std::cout<<"TODO!!!"<<std::endl;
    return true; }

  //2.2 and some user defined functions
  uint32_t baseline(){
    // the lenght of baseline is 3 bytes, so strange ~O_o~.
    uint32_t rt=0;
    rt |= this->data[6]<<16; rt |= this->data[7]<<8; rt |= this->data[8]<<16;
    return rt; }
  uint16_t amp(){
    uint16_t rt=0;
    rt |= (this->data[9]<<8) + this->data[10];
    return rt & 0x0FFF; } };

/* struct data_tail : public data_region<unsigned char,event_tail,size_t,FILE*> */ 
  // in "temp.cpp" Line 174

//3. 'Inject' all components in 2 (data_head, data_body,data_tail) to 'unpacker'
//  (see Line 470)
// unpacker<data_head,data_body_new,data_tail> opt_aa;
// opt_aa.setfile("...");
// opt_aa.parse();
// do some thing with 'opt_aa.in_memory' ...









int main(int argc, char* argv[]){
  //unpacker<data_head,data_body,data_tail> opt_aa;
  //opt_aa.setfile("RunID10021_20230914L0541_HEIC-Cube_Sci.dat");
  //opt_aa.parse();
  ////opt_aa.display();
  //auto const& to_TGraph = [](unsigned char* addr){
  //  auto* rt = new TGraph(512);
  //  for (int i=0; i<1024; i+=2){
  //    uint16_t value = 4096.f - (((addr[i]<<8) + addr[i+1]) & 0x0FFF);
  //    rt->SetPoint(i/2,i/2,value);
  //  }
  //  return rt; };

  //int i=0;
  TFile* root_fout = new TFile("temp.root","recreate");
  //for (auto&& x : opt_aa.in_memory){
  //  //std::cout<<x.first<<" "<<x.second.size()<<std::endl;
  //  if (i>=1200 && i<1220){

  //    //for (auto&& y : x.second){
  //    //  if(y->name=="body"){
  //    //    unsigned char* adc_datas = dynamic_cast<data_body*>(y)->adc;
  //    //    std::cout<<adc_datas[0]<<std::endl;
  //    //    //unsigned char* adc_datas = static_cast<data_body*>(y)->adc;
  //    //    //info_out(adc_datas);
  //    //  }
  //    //}
  //    //std::cout<<x.first<<" "
  //    //  <<x.second.m_heads.size()<<" "
  //    //  <<x.second.m_bodys.size()<<" "
  //    //  <<x.second.m_tails.size();
  //    std::stringstream sstr; sstr<<"event-"<<x.first;
  //    TFolder* f = new TFolder(sstr.str().c_str(),sstr.str().c_str());
  //    for (auto&& y : x.second.m_bodys){
  //      std::stringstream sstr;
  //      sstr<<"timeVsadc-"<<(int)y->FE_ID()<<"-"<<(int)y->channal_index();
  //      auto* g = to_TGraph(y->adc);
  //      g->SetName(sstr.str().c_str());
  //      f->Add(g);
  //    }
  //    f->Write();

  //    std::cout.put('\n');
  //  }
  //  i++;
  //  if (i>=1220) break;
  //  
  //}
  //opt_aa.clear();


  //---------------------------------------------------------------------
  unpacker<data_head,data_body_new,data_tail> opt_aa;

  std::string datname = argc>=2 ? std::string{argv[1]} 
    : "RunID66683_20240311L1549_HEIC-Cube_Sci.dat";
  opt_aa.setfile(datname);
  opt_aa.parse();

  size_t i=0;
  for (auto&& x : opt_aa.in_memory){
    if (i>500 && i<520){
      std::stringstream sstr; sstr<<"event-"<<x.first<<"-baseline";
      TH1F* f = new TH1F(sstr.str().c_str(),sstr.str().c_str(),1000,0,1000);
      for (auto&& y : x.second.m_bodys){
        std::cout<<y->baseline()<<" ";
        f->Fill(y->baseline());
      }
      f->Write();
      std::cout.put('\n');

      
    }
    if (i++>=520) break;
  }

  info_out(opt_aa.m_fname);
  info_out(opt_aa.in_memory.size());
  opt_aa.clear();

  root_fout->Write(); root_fout->Close();


  return 0;
}
//g++ -std=c++11 -pthread -Wall -fPIC -O3 -m64 -rdynamic read_new.cxx -o read `root-config --libs --cflags`
