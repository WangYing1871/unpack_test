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
#include <mutex>
#include <thread>
#include <bitset>
#include <map>
#include <unordered_map>
#include <sstream>
#include <fstream>

#ifdef ROOT_IMPORT
#include "TFolder.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1F.h"
#include "TTree.h"
//---------------------------------------------------------------------
struct root_event_raw{
  uint32_t unique_id;
  uint32_t event_id;
  double time;
  std::vector<unsigned long> adcs;

  std::ostream& display(std::ostream& os = std::cout){
    return os<<event_id<<" "<<time<<" "<<adcs.size()<<std::endl;
  }
  
};
//---------------------------------------------------------------------

#endif

struct curr_event_record{
  //TODO
  std::unordered_map<uint16_t,uint8_t> m_map;

};
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
        //info_out(ftell(fp));
    return true;
  }

  double get_time() const{
    unsigned char const* dp = this->data;
    uint64_t time_uint = 0;
    //std::cout<<std::hex;
    //for (int i=0; i<10; ++i) std::cout<<(int)dp[i]<<" ";
    //std::cout<<std::dec<<std::endl;
    time_uint |= (uint64_t)dp[4]<<40;
    time_uint |= (uint64_t)dp[5]<<32;
    time_uint |= (uint64_t)dp[6]<<24;
    time_uint |= (uint64_t)dp[7]<<16;
    time_uint |= (uint64_t)dp[8]<<8;
    time_uint |= (uint64_t)dp[9]<<0;
    //std::cout<<time_uint<<std::endl;
    return time_uint/cs_fz;
  }

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
  uint8_t channel_index() const {return data[4] & 0b01111111;}
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
    return result & 0x1FFF;
  }

  virtual bool crc32() const override {
    std::cout<<"TODO!!!"<<std::endl;
    return true;
  }

  uint32_t get_event_id() const{
    uint32_t rt{};
    rt |= this->data[4]<<24; rt |= this->data[5]<<16;
    rt |= this->data[6]<<8; rt |= this->data[7]<<0;
    return rt; }

};



template <class _tp, class _up , class _fp>
struct unpacker{

  constexpr static uint32_t s_complete_size=10;
  constexpr static uint32_t s_loop_size=0xFFFF;

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
  typedef decltype(std::begin(in_memory)) map_iter_t;
  uint32_t active_event = (std::numeric_limits<uint32_t>::max)();

  void clear(){
    for (auto&& x : in_memory) x.second.clear();
    in_memory.clear();
  }
  std::string m_fname;
  std::mutex m_mutex;

  void setfile(std::string const& v) {m_fname = v;}

  map_iter_t get_event_id(uint32_t raw_id){
    if (auto iter = in_memory.find(raw_id); iter != in_memory.end()){
      if (iter->second.m_heads.size()==iter->second.m_tails.size()){
        if (iter->second.m_heads.size()==s_complete_size) return get_event_id(raw_id+s_loop_size);
        else{
          if (true){ 
           //TODO: while distance with prev same key too long; shoule return 'get_event_id(raw_id+s_loop_size)'
            return iter;
          }else{
            return iter;
          }
        }
      }else{
       return iter;
      }
    }else{
      in_memory.insert(std::make_pair(raw_id,event_t{}));
      return in_memory.find(raw_id);
    }
  }

  void parse(){
    
    //std::string fname = "RunID66683_20240311L1549_HEIC-Cube_Sci.dat";
    auto* fp = fopen(m_fname.c_str(),"r");
    fseek(fp,0,SEEK_END); long fs = ftell(fp); fseek(fp,0,SEEK_SET);

    //std::cout.put('\t'); std::cout.put('\t');
    //std::cout.put('|');
    //for (int i=0; i<100; ++i) std::cout.put(' ');
    //std::cout.put('|');
    //std::cout<<std::endl;
    bool is_v = true;
    //std::thread viewer([&,this](){
    //    while(is_v){
    //      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //      std::lock_guard<std::mutex> lock(m_mutex);
    //      std::cout<<"\r";
    //      std::cout.put('\t'); std::cout.put('\t');

    //      for (int i=0; i<100*ftell(fp)/fs; ++i) std::cout.put('=');
    //      std::cout.put('>');
    //      std::cout<<100*ftell(fp)/(double)fs<<"%";
    //      std::cout<<std::flush;
    //    }
    //    });
    //viewer.detach();

    
    if (!fp){
      throw std::invalid_argument("a invalid input file name");
    }
    //for ( int i=0; i<10; ++i){
    //  std::cout<<fseek(fp,0,SEEK_END)<<" "<<ftell(fp)<<std::endl;
    //  std::this_thread::sleep_for(std::chrono::milliseconds(300));
    //}
    map_iter_t curr_event = std::end(in_memory);

    while(!feof(fp)){
      unsigned char data = 0;
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
          
          //in_memory[evt_id].m_heads.emplace_back(head);
          (curr_event=get_event_id(evt_id))->second.m_heads.emplace_back(head);
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
          if (curr_event != std::end(in_memory)) curr_event->second.m_bodys.emplace_back(body);
          continue;
        }
        fseek(fp,back,SEEK_SET);
        delete body;
        
        back = ftell(fp);
        auto* tail = new _fp(fp);
        if (tail->valid()){
          //if (in_memory.size()==20) break;
          //std::cout<<"tail get: "<<ftell(fp)-1<<std::endl;
          //fseek(fp,-1,SEEK_CUR);
          tail->name = "tail";
          uint64_t pos = (uint64_t)ftell(fp)-1;
          head->range[0] = pos; 
          head->range[1] = pos+tail->size();
          auto event_id = tail->get_event_id();
          get_event_id(event_id)->second.m_tails.emplace_back(tail);
          //in_memory[active_event].m_tails.emplace_back(tail);
          //std::cout<<std::hex<<ftell(fp)<<std::dec<<"\n";
          //exit(-1);
          continue;
        }

        fseek(fp,back,SEEK_SET);
        delete tail;
        //fseek(fp,-11,SEEK_CUR);

        //std::cout<<ftell(fp)<<std::endl;
      }
      //if (ftell(fp)>=50000) break;
    }
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      is_v = false;
    }
    fclose(fp);
    std::cout<<std::endl<<"\t\tW>~ _ <~W "<< std::endl;
       

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
  float baseline(){
    // the lenght of baseline is 3 bytes, so strange ~O_o~.
    uint32_t rt=0;
    rt |= this->data[6]<<16; rt |= this->data[7]<<8; rt |= this->data[8];
    return 4096.f-rt/128.f;
  }

  uint32_t baseline_u32(){
    uint32_t rt=0;
    rt |= this->data[6]<<16; rt |= this->data[7]<<8; rt |= this->data[8];
    return rt;
  }
  uint16_t amp(){
    uint16_t rt=0;
    rt |= (this->data[9]<<8) + this->data[10];
    return 4096.f -(rt & 0x0FFF);
  }

  uint8_t FE_ID() const{ return data[3] & 0b00111111; }
  uint8_t channel_index() const {return data[4] & 0b01111111;}
};

/* struct data_tail : public data_region<unsigned char,event_tail,size_t,FILE*> */ 
  // in "temp.cpp" Line 174

//3. 'Inject' all components in 2 (data_head, data_body,data_tail) to 'unpacker'
//  (see Line 470)
// unpacker<data_head,data_body_new,data_tail> opt_aa;
// opt_aa.setfile("...");
// opt_aa.parse();
// do some thing with 'opt_aa.in_memory' ...







int main(int argc, char* argv[]){

  unpacker<data_head,data_body_new,data_tail> opt_aa;

  std::string datname = argc>=2 ? std::string{argv[1]} 
    : "RunID66683_20240311L1549_HEIC-Cube_Sci.dat";
  opt_aa.setfile(datname);
  opt_aa.parse();


  //std::ofstream fout("aaa.txt");

  //std::size_t index = 0;
  //size_t low_bound = 0;
  //size_t up_bound = opt_aa.in_memory.size();

  //index = 0;
  //for (auto iter = opt_aa.in_memory.begin(); iter != opt_aa.in_memory.end(); ++iter){
  //  if (index>=low_bound && index<up_bound){
  //    fout<<iter->first<<" "<<iter->second.m_heads.size()<<" "
  //      <<iter->second.m_bodys.size()<<" "<<iter->second.m_tails.size()<<std::endl;
  //    for (auto&& y : iter->second.m_heads) fout<<y->get_time()<<" ";
  //    for (auto&& y : iter->second.m_bodys){
  //      fout<<"\t"<<(int)y->FE_ID()<<" "<<(int)y->channel_index()<<" "<<(int)y->baseline_u32()<<" "<<y->amp()<<"\n";
  //    }
  //    fout<<"\n";
  //  }else if (index>up_bound)
  //    break;
  //  index++;
  //}




  //fout.close();





#ifdef ROOT_IMPORT
  {
    std::string root_raw_name = datname.substr(0,datname.find_last_of('.'))+".root";
    TFile* root_fout = new TFile(root_raw_name.c_str(),"recreate");

    struct root_event_raw root_event_raw;
    TTree* atree = new TTree("data","data");
    atree->Branch("event_id",&root_event_raw.event_id);
    atree->Branch("unique_id",&root_event_raw.unique_id);
    atree->Branch("time",&root_event_raw.time);
    atree->Branch("adcs",&root_event_raw.adcs);
    info_out(atree);
    
    auto const& to_rootraw_struct = []<class iter_t>
      (iter_t from_bgn, iter_t from_end, struct root_event_raw& to){
      to.adcs.clear();
      for (auto iter = from_bgn; iter != from_end; ++iter){
        unsigned long tmp = 0;
        auto& value = **iter;
        tmp |= (unsigned long)value.FE_ID()<<48;
        tmp |= (unsigned long)value.channel_index()<<40;
        tmp |= (unsigned long)value.baseline_u32()<<16;
        tmp |= (unsigned long)value.amp();
        to.adcs.emplace_back(tmp); } };


    std::ofstream fout("aaa.txt");
    info_out("====");

    for (auto&& x : opt_aa.in_memory){
      if (x.second.m_heads.size() != x.second.m_tails.size()
          || x.second.m_heads.size()<1) continue;
      //std::cout<<x.first<<" "<<x.second.m_heads.size()<<" "<<x.second.m_bodys.size()<<" "<<x.second.m_tails.size()<<std::endl;
      root_event_raw.unique_id = x.first;
      root_event_raw.event_id = x.second.m_heads[0]->get_event_id();
      root_event_raw.time = x.second.m_heads[0]->get_time();
      auto& range = x.second.m_bodys;
      to_rootraw_struct(range.begin(),range.end(),root_event_raw);
      root_event_raw.display(fout);
      atree->Fill();
    }

    fout.close();

//    for (auto&& x : opt_aa.in_memory){
//      if (x.second.m_heads.size() != x.second.m_tails.size()
//          || x.second.m_heads.size()<1) continue;
//      info_out("====");
//      //root_event_raw.unique_id = x.first;
//      //root_event_raw.event_id = x.second.m_heads[0]->get_event_id();
//      //root_event_raw.time = x.second.m_heads[0]->get_time();
//      //auto& range = x.second.m_bodys;
//      //to_rootraw_struct(range.begin(),range.end(),root_event_raw);
//      //root_event_raw.display();
//      //atree->Fill();
//      //break;
//    }



    //for (auto&& x : opt_aa.in_memory){
    //  std::cout<<x.second.m_heads.size()<<" "<<x.second.m_tails.size()<<std::endl;
    //}

    //size_t i=0;
    //for (auto&& x : opt_aa.in_memory){
    //  if (i>500 && i<520){
    //    std::stringstream sstr; sstr<<"event-"<<x.first<<"-baseline";
    //    TH1F* f = new TH1F(sstr.str().c_str(),sstr.str().c_str(),1000,0,1000);
    //    for (auto&& y : x.second.m_bodys){
    //      std::cout<<y->baseline()<<" ";
    //      f->Fill(y->baseline());
    //    }
    //    f->Write();
    //    std::cout.put('\n');

    //    
    //  }
    //  if (i++>=520) break;
    //}

    //info_out(opt_aa.m_fname);
    //info_out(opt_aa.in_memory.size());
    opt_aa.clear();
    atree->Write();

    root_fout->Write(); root_fout->Close();
  }
#endif

#ifdef TEST_0
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  {
    typedef struct{
      struct channel_t{
        uint8_t FE_id;
        uint8_t channel_index;
        uint32_t baseline;
        uint16_t amp;
      };

      void load_channel(unsigned long from){
        channel_t info;
        info.FE_id = (from>>48); info.channel_index = (from>>40) & 0xFF;
        info.baseline = (from>>16) & 0xFFFFFF; info.amp = from & 0xFFFF;
        channels.emplace_back(info);
      }
      std::ostream& display_channels(std::ostream& os = std::cout){
        for (auto&& x : channels){
          os <<(int)x.FE_id<<" "
            <<(int)x.channel_index<<" "
            <<(4096.f-x.baseline/128)<<" "
            <<(4096.f-x.amp)<<" "
            <<"\n";
        }
        return os;
      }
      uint32_t event_id;
      double time;
      std::vector<channel_t> channels;
    } event_info_t;

    auto* fin = new TFile(root_raw_name.c_str());
    auto* data_tree = static_cast<TTree*>(fin->Get("data"));
    //std::cout<<data_tree<<std::endl; exit(0);

    event_info_t event_info;

    data_tree->SetBranchAddress("event_id",&event_info.event_id);
    data_tree->SetBranchAddress("time",&event_info.time);
    std::vector<unsigned long>* aplace;
    data_tree->SetBranchAddress("adcs",&aplace);

    auto const& to_event_info = [&aplace,&event_info](TTree* tr, int64_t i){
      tr->GetEntry(i);
      event_info.channels.clear();
      for (auto&& x : aplace[0])  event_info.load_channel(x);
    };

    for (int i=0; i<data_tree->GetEntries(); ++i){
      to_event_info(data_tree,i);
      event_info.display_channels();
      std::cout<<"================================="<<std::endl;
    }

    fin->Close();
  }
#endif

  return 0;
}
//g++ -std=c++11 -pthread -Wall -fPIC -O3 -m64 -rdynamic read_new.cxx -o read -DROOT_IMPORT `root-config --libs --cflags`
