#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <cstdint>
#include <optional>
#include <functional>
#include <stdexcept>

class asm_65c02
{
  private:
    struct s_opcode
    {
      std::string line;
      std::string arg1;
      std::string arg2;
      size_t size;
      int32_t pos;
      uint32_t line_n;
      std::optional<uint8_t> operand1;
      std::optional<uint8_t> operand2;
      uint8_t opcode;
      uint8_t mode;
    };
    struct s_label 
    {
      uint32_t line_n;
      std::optional<int32_t> val;
    };
    struct s_var
    {
      std::string arg1;
      std::string line;
      std::optional<int32_t> val;
      uint32_t line_n;
    };
    struct s_byte
    {
      std::string line;
      std::string arg1;
      std::vector<uint8_t> vector;
      uint32_t line_n;
      int32_t pos;
      uint8_t mode;
    };
    struct s_line
    {
      std::string line;
      uint32_t line_n;
    };
    struct s_ascii 
    {
      std::string line;
      int32_t pos;
    };
    int argc;
    int32_t org{0x8000};
    int32_t org_start{0x8000};
    int32_t org_end{0x8000};
    char** argv;
    const std::unordered_set<std::string> ins_r
    {
      "bcc", "bcs", "beq", "bmi", "bne", "bpl", "bra", "bvc", "bvs"
    };
    const std::unordered_map<std::string, uint8_t> opcode_map
    {
      {"brk_i",0x00}, {"ora_(zp,x)",0x01}, {"tsb_zp",0x04}, {"ora_zp",0x05}, {"asl_zp",0x06},
      {"rmb0_zp",0x07}, {"rmb1_zp",0x17}, {"rmb2_zp",0x27}, {"rmb3_zp",0x37}, {"rmb4_zp",0x47}, {"rmb5_zp",0x57}, {"rmb6_zp",0x67} ,{"rmb7_zp",0x77}, {"php_i",0x08},
      {"ora_#", 0x09}, {"asl_A", 0x0a}, {"tsb_a", 0x0c}, {"ora_a",0x0d}, {"asl_a",0x0e}, {"bbr0_zp,r",0x0f}, 
      {"bbr1_zp,r",0x1f}, {"bbr2_zp,r",0x2f}, {"bbr3_zp,r",0x3f}, {"bbr4_zp,r",0x4f}, {"bbr5_zp,r",0x5f}, {"bbr6_zp,r",0x6f}, {"bbr7_zp,r",0x7f}, 
      {"bbs0_zp,r",0x8f}, {"bbs1_zp,r",0x9f}, {"bbs2_zp,r",0xaf}, {"bbs3_zp,r",0xbf}, {"bbs4_zp,r",0xcf}, {"bbs5_zp,r",0xdf}, {"bbs6_zp,r",0xef}, {"bbs7_zp,r",0xff}, 
      {"bpl_r",0x10}, {"ora_(zp),y",0x11}, {"ora_(zp)",0x12}, {"trb_zp",0x14}, {"ora_zp,x",0x15}, {"asl_zp,x",0x16}, {"clc_i",0x18}, {"ora_a,y",0x19},
      {"inc_A",0x1a}, {"trb_a",0x1c}, {"ora_a,x",0x1d}, {"asl_a,x",0x1e}, {"jsr_a",0x20}, {"and_(zp,x)",0x21}, {"bit_zp",0x24},
      {"and_zp",0x25}, {"rol_zp",0x26}, {"plp_i",0x28}, {"and_#",0x29}, {"rol_A",0x2a}, {"bit_a",0x2c}, {"and_a",0x2d}, {"rol_a",0x2e}, {"bmi_r",0x30},
      {"and_(zp),y",0x31}, {"and_(zp)",0x32},  {"bit_zp,x",0x34}, {"and_zp,x",0x35}, {"rol_zp,x",0x36}, {"sec_i",0x38}, {"and_a,y",0x39}, {"dec_A",0x3a},
      {"bit_a,x",0x3c}, {"and_a,x",0x3d}, {"rol_a,x",0x3e}, {"rti_i",0x40}, {"eor_(zp,x)",0x41}, {"eor_zp",0x45}, {"lsr_zp",0x46}, {"pha_i",0x48}, {"eor_#",0x49},
      {"lsr_A",0x4a}, {"jmp_a",0x4c}, {"eor_a",0x4d}, {"lsr_a",0x4e}, {"bvc_r",0x50}, {"eor_(zp),y",0x51}, {"eor_(zp)",0x52}, {"eor_zp,x",0x55}, {"lsr_zp,x",0x56},
      {"cli_i",0x58}, {"eor_a,y",0x59}, {"phy_i",0x5a}, {"eor_a,x",0x5d}, {"lsr_a,x",0x5e}, {"rts_i",0x60}, {"adc_(zp,x)",0x61}, {"stz_zp",0x64}, {"adc_zp",0x65},
      {"ror_zp",0x66}, {"pla_i",0x68}, {"adc_#",0x69}, {"ror_A",0x6a}, {"jmp_(a)",0x6c}, {"adc_a",0x6d}, {"ror_a",0x6e}, {"bvs_r",0x70}, {"adc_(zp),y",0x71},
      {"adc_(zp)",0x72}, {"stz_zp,x",0x74}, {"adc_zp,x",0x75}, {"ror_zp,x",0x76}, {"sei_i",0x78}, {"adc_a,y",0x79}, {"ply_i",0x7a}, {"jmp_(a,x)",0x7c}, {"adc_a,x",0x7d},
      {"ror_a,x",0x7e}, {"bra_r",0x80}, {"sta_(zp,x)",0x81}, {"sty_zp",0x84}, {"sta_zp",0x85}, {"stx_zp",0x86}, {"dey_i",0x88}, {"bit_#",0x89}, {"txa_i",0x8a},
      {"sty_a",0x8c}, {"sta_a",0x8d}, {"stx_a",0x8e}, {"bcc_r",0x90}, {"sta_(zp),y",0x91}, {"sta_(zp)",0x92}, {"sty_zp,x",0x94}, {"sta_zp,x",0x95}, {"stx_zp,y",0x96},
      {"tya_i",0x98}, {"sta_a,y",0x99}, {"txs_i",0x9a}, {"stz_a",0x9c}, {"sta_a,x",0x9d}, {"stz_a,x",0x9e}, {"ldy_#",0xa0}, {"lda_(zp,x)",0xa1}, {"ldx_#",0xa2},
      {"ldy_zp",0xa4}, {"lda_zp",0xa5}, {"ldx_zp",0xa6}, {"tay_i",0xa8}, {"lda_#",0xa9}, {"tax_i",0xaa}, {"ldy_a",0xac}, {"lda_a",0xad}, {"ldx_a",0xae},
      {"bcs_r",0xb0}, {"lda_(zp),y",0xb1}, {"lda_(zp)",0xb2}, {"ldy_zp,x",0xb4}, {"lda_zp,x",0xb5}, {"ldx_zp,y",0xb6}, {"clv_i",0xb8}, {"lda_a,y",0xb9}, {"tsx_i",0xba},
      {"ldy_a,x",0xbc}, {"lda_a,x",0xbd}, {"ldx_a,y",0xbe}, {"cpy_#",0xc0}, {"cmp_(zp,x)",0xc1}, {"cpy_zp",0xc4}, {"cmp_zp",0xc5}, {"dec_zp",0xc6}, {"iny_i",0xc8},
      {"cmp_#",0xc9}, {"dex_i",0xca}, {"wai_i",0xcb}, {"cpy_a",0xcc}, {"cmp_a",0xcd}, {"dec_a",0xce}, {"bne_r",0xd0}, {"cmp_(zp),y",0xd1}, {"cmp_(zp)",0xd2},
      {"cmp_zp,x",0xd5}, {"dec_zp,x",0xd6}, {"cld_i",0xd8}, {"cmp_a,y",0xd9}, {"phx_i",0xda}, {"stp_i",0xdb},{"cmp_a,x",0xdd},{"dec_a,x",0xde},{"cpx_#",0xe0},{"sbc_(zp,x)",0xe1},
      {"cpx_zp",0xe4}, {"sbc_zp",0xe5}, {"inc_zp",0xe6}, {"inx_i",0xe8}, {"sbc_#",0xe9}, {"nop_i",0xea}, {"cpx_a",0xec}, {"sbc_a",0xed}, {"inc_a",0xee},
      {"beq_r",0xf0}, {"sbc_(zp),y",0xf1}, {"sbc_(zp)",0xf2}, {"sbc_zp,x",0xf5}, {"inc_zp,x",0xf6}, {"sed_i",0xf8}, {"sbc_a,y",0xf9}, {"plx_i",0xfa}, {"sbc_a,x",0xfd},
      {"inc_a,x",0xfe}, {"smb0_zp",0x87}, {"smb1_zp",0x97},{"smb2_zp",0xa7},{"smb3_zp",0xb7},{"smb4_zp",0xc7},{"smb5_zp",0xd7},{"smb6_zp",0xe7},{"smb7_zp",0xf7},
    };
    std::vector<s_line> in_vector;
    std::vector<char> out_vector;
    std::vector<s_opcode> opcode_vector;
    std::vector<uint32_t> opcode_index_vector;
    std::vector<s_byte> dir_byte_vector;
    std::vector<s_ascii> dir_ascii_vector;
    std::unordered_map<std::string, s_var> var_map;
    std::unordered_map<std::string, s_label> label_map;
    std::stringstream ss;
    std::smatch m;
    const std::array<std::regex, 3> rx_dir
    {
      std::regex(R"(^\s*\.byte\s+(.+)$)"),
      std::regex(R"(^\s*\.word\s+(.+)$)"),
      std::regex(R"(^\s*\.ascii\s+\"(.*)\"\s*$)"),
    };
    const std::array<std::regex, 12> rx_addr_mode
    {
      //A
      std::regex(R"(^\s*(\w+)\s+[aA]\s*$)"),
      //#
      std::regex(R"(^\s*(\w+)\s+#(-?[$%]?[^\s$%:.*#,()=+-]+\s*(?:[+-]\s*-?[$%]?[^\s$%:.*#,()=+-]+\s*)*)$)"),
      //a & zp
      std::regex(R"(^\s*(\w+)\s+(-?[$%]?[^\s$%:.*#,()=+-]+\s*(?:[+-]\s*-?[$%]?[^\s$%:.*#,()=+-]+\s*)*)$)"),
      //a,x & zp,x
      std::regex(R"(^\s*(\w+)\s+(-?[$%]?[^\s$%:.*#,()=+-]+\s*(?:[+-]\s*-?[$%]?[^\s$%:.*#,()=+-]+\s*)*),\s*[xX]\s*$)"),
      //a,y & zp,y
      std::regex(R"(^\s*(\w+)\s+(-?[$%]?[^\s$%:.*#,()=+-]+\s*(?:[+-]\s*-?[$%]?[^\s$%:.*#,()=+-]+\s*)*),\s*[yY]\s*$)"),
      //(a) & (zp)
      std::regex(R"(^\s*(\w+)\s+\(\s*(-?[$%]?[^\s$%:.*#,()=+-]+\s*(?:[+-]\s*-?[$%]?[^\s$%:.*#,()=+-]+\s*)*)\)\s*$)"),
      //(a,x) & (zp,x)
      std::regex(R"(^\s*(\w+)\s+\(\s*(-?[$%]?[^\s$%:.*#,()=+-]+\s*(?:[+-]\s*-?[$%]?[^\s$%:.*#,()=+-]+\s*)*),\s*[xX]\s*\)\s*$)"),
      //(zp),y
      std::regex(R"(^\s*(\w+)\s+\(\s*(-?[$%]?[^\s$%:.*#,()=+-]+\s*(?:[+-]\s*-?[$%]?[^\s$%:.*#,()=+-]+\s*)*)\)\s*,\s*[yY]\s*$)"),
      //zp,r
      std::regex(R"(^\s*(\w+)\s+(-?[$%]?[^\s$%:.*#,()=+-]+\s*(?:[+-]\s*-?[$%]?[^\s$%:.*#,()=+-]+\s*)*),\s*(-?[$%]?[^\s$%:.*#,()=+-]+\s*(?:[+-]\s*-?[$%]?[^\s$%:.*#,()=+-]+\s*)*)\s*$)"),
      //i
      std::regex(R"(^\s*(\w+)\s*$)"),
      //# label
      std::regex(R"(^\s*(\w+)\s+#([lL][oO]|[hH][iI])\s+(-?[$%]?[^\s$%:.*#,()=+-]+\s*(?:[+-]\s*-?[$%]?[^\s$%:.*#,()=+-]+\s*)*)$)"),
      std::regex(R"(^\s*(\w+)\s+\*(-?[$%]?[^\s$%:.*#,()=+-]+\s*(?:[+-]\s*-?[$%]?[^\s$%:.*#,()=+-]+\s*)*)$)"),
    };
    const std::regex rx_var{R"(^\s*([^0-9$%:.*#,()=+-][^\s$%:.*#,()=+-]*)\s*=\s*(-?[$%]?[^\s$%:.*#,()=+-]+\s*(?:[+-]\s*-?[$%]?[^\s$%:.*#,()=+-]+\s*)*)$)"};
    const std::regex rx_label{R"(^\s*([^0-9$%:.*#,()=+-][^\s$%:.*#,()=+-]*):\s*$)"};
    const std::regex rx_org{R"(^\s*\.org\s*(-?[$%]?[^\s$%:.*#,()=+-]+\s*(?:[+-]\s*-?[$%]?[^\s$%:.*#,()=+-]+\s*)*)$)"};
    const std::regex rx_include{R"(^\s*\.include\s+\"(.*)\"\s*$)"};
    const std::regex rx_other{R"(^\s*(-?[$%]?[^\s$%:.*#,()=+-]+\s*(?:[+-]\s*-?[$%]?[^\s$%:.*#,()=+-]+\s*)*)$)"};
    const std::regex rx_space{R"(\s+)"};
    const std::regex rx_hex{R"([^0-9a-fA-F]+)"};
    const std::regex rx_binary{R"([^0-1]+)"};
    const std::regex rx_decimal{R"([^0-9]+)"};
    const std::regex rx_comma{R"([^,]+)"};
    const std::unordered_map<uint8_t, void(asm_65c02::*)(s_opcode& opcode)> mode_pfn_map
    {
      {relative_, &asm_65c02::relative},
      {absolute_, &asm_65c02::absolute},
      {zero_page_, &asm_65c02::zero_page},
      {immediate_, &asm_65c02::zero_page},
      {zero_page_relative_, &asm_65c02::zero_page_relative},
      {immediate_label_, &asm_65c02::immediate_label}
    };
    static constexpr uint8_t relative_{0};
    static constexpr uint8_t absolute_{1};
    static constexpr uint8_t zero_page_{2};
    static constexpr uint8_t immediate_{3};
    static constexpr uint8_t zero_page_relative_{4};
    static constexpr uint8_t immediate_label_{5};
    static constexpr uint8_t label{1};
    static constexpr uint8_t negate{2};
    static constexpr uint8_t zp{4};
    static constexpr uint8_t label_opt{8};
    bool error{};
  private:
    void read_file(const std::string file)
    {
      std::ifstream in(file);
      if(!in.is_open())
      {
        throw std::invalid_argument(file + " does not exist");
      }
      std::string line;
      uint32_t i{};
      while(std::getline(in, line))
      {
        i++;
        remove_comments(line);
        if(line.empty() || std::regex_match(line, rx_space))
          continue;
        else if(std::regex_match(line, m, rx_include))
        {
          read_file(m[1].str());
          continue;
        }
        in_vector.emplace_back(s_line{line,i});
      }
    }
    void remove_comments(std::string& line)
    {
      if(size_t pos = line.find("//"); pos != std::string::npos)
      {
        if(size_t pos2 = line.find_first_of("\""); pos2 != std::string::npos)
        {
          if(pos < pos2)
          {
            line.erase(pos);
            return;
          }
          if(size_t pos3 = line.find_last_of("\""); pos3 != pos2)
          {
            std::string s = line;
            s.erase(pos2, pos3 - pos2 + 1);
            if(size_t pos4 = s.find("//"); pos4 != std::string::npos)
              line.erase(pos4 + pos3 - pos2 + 1);
          }
        }
        else 
          line.erase(pos);
      }
    }
    void set_org(const std::string& line, const uint32_t& line_n, std::string& arg1)
    {
      std::optional<int32_t> x = calc_val(arg1, line, line_n, 0);
      if(!x.has_value())
        return;
      org = x.value();
    }
    void write()
    {
      size_t file_size = org_end - org_start;
      out_vector.resize(file_size);
      for(const s_opcode& opcode : opcode_vector)
      {
        out_vector[opcode.pos - org_start] = opcode.opcode;
        if(opcode.operand1.has_value())
          out_vector[opcode.pos - org_start + 1] = opcode.operand1.value();
        if(opcode.operand2.has_value())
          out_vector[opcode.pos - org_start + 2] = opcode.operand2.value();
      }
      for(s_byte& x : dir_byte_vector)
      {
        dir_byte(&x);
        std::copy(x.vector.begin(), x.vector.end(), out_vector.begin() + x.pos - org_start);
      }
      for(const auto& [x,y] : dir_ascii_vector)
        std::copy(x.begin(), x.end(), out_vector.begin() + y - org_start);
      if(error)
        throw std::invalid_argument(ss.str());
      std::ofstream out(argv[2], std::ios::binary);
      out.write(&out_vector[0], file_size);
    }
    std::string get_abs_mode(const uint32_t& i)
    {
      switch(i)
      {
        case 2:
          return("_a");
        case 3:
          return ("_a,x");
        case 4:
          return ("_a,y");
        case 5:
          return ("_(a)");
        case 6:
          return ("_(a,x)");
      }
      return ("");
    }
    std::string get_zp_mode(const uint32_t& i)
    {
      switch(i)
      {
        case 2:
          return("_zp");
        case 3:
          return ("_zp,x");
        case 4:
          return ("_zp,y");
        case 5:
          return ("_(zp)");
        case 6:
          return ("_(zp,x)");
      }
      return ("");
    }
    s_opcode get_opcode(const uint32_t& i, const std::string& line, const uint32_t& line_n)
    {
      std::string arg0 = m[1].str();
      std::string arg1;
      if(i != 9)
        arg1 = m[2].str();
      to_lower(arg0);
      s_opcode opcode
      {
        line,
        arg1,
        "",
        0,
        org,
        line_n,
        std::nullopt,
        std::nullopt,
        2,
        0xff
      };
      opcode.arg1 = arg1;
      if(i == 2)
      {
        if(arg0 == "jmp" || arg0 == "jsr")
        {
          opcode.mode = absolute_;
          set_opcode(arg0 + get_abs_mode(i), 3, opcode);
          return opcode;
        }
        else if(ins_r.find(arg0) != ins_r.end())
        {
          set_opcode(arg0 + "_r", 2 ,opcode);
          std::optional<int32_t> x = calc_val(arg1, line, line_n, label_opt);
          if(!x.has_value())
            opcode.mode = relative_;
          else 
          {
            check_branch(x.value(), 0, opcode, 1);
            opcode.operand1 = x;
          }
          return opcode;
        }
      }
      if(i >= 2 && i <= 6)
      {
        std::optional<int32_t> x = calc_val(arg1, line, line_n, label);
        if(!x.has_value())
        {
          opcode.mode = absolute_;
          set_opcode(arg0 + get_abs_mode(i), 3, opcode);
        }
        else if(x > 0xff || x < -0x80)
        {
          set_opcode(arg0 + get_abs_mode(i), 3, opcode);
          opcode.operand1 = x;
          opcode.operand2 = x.value() >> 8;
        }
        else 
        {
          set_opcode(arg0 + get_zp_mode(i), 2, opcode);
          opcode.operand1 = x;
        }
      }
      switch(i)
      {
        case 0:
        {
          set_opcode(arg0 + "_A", 1, opcode);
          return opcode;
        }
        case 1:
        {
          set_opcode(arg0 + "_#", 2, opcode);
          opcode.mode = immediate_;
          return opcode;
        }
        case 7:
        {
          set_opcode(arg0 + "_(zp),y", 2, opcode);
          opcode.mode = zero_page_;
          return opcode;
        }
        case 8:
        {
          set_opcode(arg0 + "_zp,r", 3, opcode);
          opcode.mode = zero_page_relative_;
          opcode.arg2 = m[3].str();
          return opcode;
        }
        case 9:
        {
          set_opcode(arg0 + "_i", 1, opcode);
          return opcode;
        }
        case 10:
        {
          set_opcode(arg0 + "_#", 2, opcode);
          opcode.arg2 = m[3].str();
          opcode.mode = immediate_label_;
          return opcode;
        }
        case 11:
        {
          std::optional<int32_t> x = calc_val(arg1, line, line_n, zp);
          set_opcode(arg0 + "_r", 2, opcode);
          opcode.operand1 = x;
          return opcode;
        }
      }
      return opcode;
    }
    std::optional<int32_t> get_val(const std::string& val, const std::string& line, const uint32_t& line_n, const uint8_t& mode)
    {
      std::optional<int32_t> x;
      std::unordered_map<std::string, s_var>::const_iterator it;
      std::unordered_map<std::string, s_label>::const_iterator it2;
      if((val[0] >= '0' && val[0] <= '9') || (val[0] == '$' || val[0] == '%'))
      {
        x = get_base(val, line, line_n);
      }
      else if(it = var_map.find(val); it != var_map.end())
      {
        x = it->second.val;
      }
      else if(it2 = label_map.find(val); it2 != label_map.end())
      {
        if(mode & label_opt)
          return std::nullopt;
        else if(mode & label)
          x = it2->second.val;
      }
      else 
      {
        x = get_base(val, line, line_n);
      }
      if(!x.has_value())
        return std::nullopt;
      if(mode & negate)
      {
        x = -x.value();
      }
      return x;
    }
    std::optional<int32_t> calc_val(std::string& arg1, const std::string& line, const uint32_t& line_n, uint8_t mode)
    {
      arg1 = std::regex_replace(arg1, rx_space, "");
      size_t pos{};
      size_t p{};
      bool y{};
      std::optional<int32_t> z;
      std::string arg;
      bool exit{};
      while(!exit)
      {
        if(!y)
        {
          pos = arg1.find_first_of("+-");
          if(pos == 0)
          {
            mode |= negate;
            p++;
            pos = arg1.find_first_of("+-", p);
          }
          arg = arg1.substr(p, pos - p);
          if(pos == std::string::npos)
          {
            z = get_val(arg,line,line_n,mode);
            if(!(mode & (label | label_opt)) && !z.has_value())
              add_to_ss(line_n, line + " (labels are not allowed here)");
            if(z.has_value())
              check_val(line, line_n, z.value(), mode);
            return z;
          }
          std::optional<int32_t> x = get_val(arg, line, line_n, mode);
          if(!x.has_value())
            return std::nullopt;
          z = x.value();
          y = 1;
        }
        mode &= ~negate;
        char ch = arg1[pos];
        std::optional<int32_t> x;
        p = pos+1;
        pos = arg1.find_first_of("+-",p);
        if(pos == p)
        {
          mode |= negate;
          p++;
          pos = arg1.find_first_of("+-", p);
        }
        if(pos == std::string::npos)
        {
          arg = arg1.substr(p);
          exit = 1;
        }
        else 
          arg = arg1.substr(p, pos - p);
        x = get_val(arg, line, line_n, mode);
        if(!(mode & (label | label_opt)) && !z.has_value())
          add_to_ss(line_n, line + " (labels are not allowed here)");
        if(!x.has_value())
          return std::nullopt;
        if(ch == '+')
          z.value() += x.value();
        else if(ch == '-')
          z.value() -= x.value();
      }
      check_val(line, line_n, z.value(), mode);
      return z;
    }
    void check_val(const std::string& line, const uint32_t& line_n, int32_t& val, const uint8_t& mode)
    {
      if((mode & zp) && (val > 0xff || val < -0x80))
      {
        add_to_ss(line_n, line + " (value exceeds 255)");
        val = 0xff;
        return;
      }
      else if(val > 0xffff || val < -0x8000)
      {
        add_to_ss(line_n, line + " (value exceeds 65535)");
        val = 0xffff;
      }
    }
    void set_opcode(const std::string& s, uint8_t size, s_opcode& opcode)
    {
      auto it = opcode_map.find(s);
      if(it != opcode_map.end())
      {
        opcode.opcode = it->second;
        opcode.size = size;
      }
    }
    void loop()
    {
      for(size_t i{}; i < in_vector.size(); i++)
      {
        bool res{};
        std::string& line = in_vector[i].line;
        const uint32_t& line_n = in_vector[i].line_n;
        if((res = org_(line,line_n))){}
        else if((res = opcodes_(line,line_n))){}
        else if((res = labels_(line))){}
        else if((res = variables_(line))){}
        else if((res = dir_(line,line_n))){}
        if(!res)
          add_to_ss(line_n, line + " (invalid syntax)");
      if(org >= org_end)
        org_end = org;
      if(org <= org_start)
        org_start = org;
      }
    }
    bool org_(const std::string& line, const uint32_t& line_n)
    {
      if(std::regex_match(line, m, rx_org))
      {
        std::string arg1 = m[1].str();
        set_org(line, line_n, arg1);
        return 1;
      }
      return 0;
    }
    bool opcodes_(const std::string& line, const uint32_t& line_n)
    {
      uint32_t size{static_cast<uint32_t>(rx_addr_mode.size())};
      for(uint32_t x{}; x < size; x++)
      {
        if(std::regex_match(line, m, rx_addr_mode[x]))
        {
          s_opcode opcode = get_opcode(x, line, line_n);
          if(opcode.opcode == 2)
          {
            add_to_ss(line_n, line + " (invalid opcode)");
          }
          if(opcode.mode != 0xff)
            opcode_index_vector.emplace_back(opcode_vector.size());
          org += opcode.size;
          opcode_vector.emplace_back(std::move(opcode));
          return 1;
        }
      }
      return 0;
    }
    bool labels_(const std::string& line)
    {
      if(std::regex_match(line, m, rx_label))
      {
        std::string s = m[1].str();
        label_map.find(s)->second.val = org;
        return 1;
      }
      return 0;
    }
    bool variables_(const std::string& line)
    {
      if(std::regex_match(line,rx_var))
        return 1;
      return 0;
    }
    bool dir_(const std::string& line, const uint32_t& line_n)
    {
      uint8_t size = static_cast<uint8_t>(rx_dir.size());
      for(uint8_t x{}; x < size; x++)
      {
        if(std::regex_match(line, m, rx_dir[x]))
        {
          std::string arg1{m[1].str()};
          if(x < 2)
            dir_byte(line, line_n, arg1, x);
          else 
            dir_ascii(line, line_n, arg1);
          return 1;
        }
      }
      return 0;
    }
    void dir_byte(const std::string& line, const uint32_t& line_n, const std::string& arg1, const uint8_t& mode)
    {
      const int32_t pos{org};
      std::sregex_iterator it(arg1.begin(), arg1.end(), rx_comma);
      std::sregex_iterator end{};
      for(;it != end; it++)
      {
        org++;
        if(mode)
          org++;
        }
      dir_byte_vector.emplace_back(s_byte{line, arg1, {}, line_n, pos, mode});
    }
    void dir_byte(s_byte* data)
    {
      std::sregex_iterator it(data->arg1.begin(), data->arg1.end(), rx_comma);
      std::sregex_iterator end{};
      for(;it != end; it++)
      {
        std::string s{it->str()};
        if(std::regex_match(s,rx_other))
        {
          std::optional<int32_t> x;
          if(!data->mode)
            x = calc_val(s, data->line, data->line_n, label | zp);
          else 
            x = calc_val(s, data->line, data->line_n, label);
          if(!x.has_value())
          {
            add_to_ss(data->line_n, data->line + " (error)");
            return;
          }
          data->vector.emplace_back(x.value());
          if(data->mode)
            data->vector.emplace_back(x.value() >> 8);
        }
        else 
        {
          add_to_ss(data->line_n, data->line + " (parsing error)");
          return;
        }
      }
    }
    void dir_ascii(const std::string& line, const uint32_t& line_n, std::string& arg1)
    {
      if(parse_ascii(line, line_n, arg1))
      {
        dir_ascii_vector.emplace_back(s_ascii{arg1,org});
        org += arg1.size();
      return;
      }
    }
    bool parse_ascii(const std::string& line, const uint32_t& line_n, std::string& s)
    {
      bool x{};
      size_t i{};
      std::string::const_iterator it = s.begin();
      std::string::const_iterator it_end = s.end();
      for(;it != it_end; it++)
      {
        if(!x && *it == '\\')
        {
          x = 1;
          if(it + 1 == it_end)
          {
            add_to_ss(line_n, line + " (unknown escape sequence)");
            return 0;
          }
          continue;
        }
        if(x)
        {
          switch(*it)
          {
            case '\\':
              s[i] = '\\';
              break;
            case 'a':
              s[i] = '\a';
              break;
            case 'b':
              s[i] = '\b';
              break;
            case 'f':
              s[i] = '\f';
              break;
            case 'n':
              s[i] = '\n';
              break;
            case 'r':
              s[i] = '\r';
              break;
            case 't':
              s[i] = '\t';
              break;
            case 'v':
              s[i] = '\v';
              break;
            default:
              add_to_ss(line_n, line + " (unknown escape sequence)");
              return 0;
          }
          x = 0;
          i++;
          continue;
        }
        s[i] = *it;
        i++;
      }
      if(s.size() > i)
      {
        s.resize(i);
      }
      return 1;
    }
    void labels()
    {
      for(auto& [x,y] : var_map)
      {
        if(!y.val.has_value())
        {
          std::optional<int32_t> z = calc_val(y.arg1, y.line, y.line_n, label);
          y.val = z;
        }
      }
      for(const uint32_t& i : opcode_index_vector)
      {
        s_opcode& opcode = opcode_vector[i];
        std::invoke(mode_pfn_map.find(opcode.mode)->second, this, opcode);
      }
    }
    void check_branch(const int& x, const int& y, const s_opcode& opcode, bool mode)
    {
      if(x - y < -0x80)
        add_to_ss(opcode.line_n, opcode.line + " (out of range branch)");
      else if(!mode && x - y > 0x7f)
        add_to_ss(opcode.line_n, opcode.line + " (out of range branch)");
      else if(x - y > 0xff)
        add_to_ss(opcode.line_n, opcode.line + " (out of range branch)");
    }
    void relative(s_opcode& opcode)
    {
      std::optional<int32_t> x = calc_val(opcode.arg1, opcode.line, opcode.line_n, label);
      check_branch(x.value(), opcode.pos + opcode.size, opcode, 0);
      opcode.operand1 = x.value() - opcode.pos - opcode.size;
    }
    void absolute(s_opcode& opcode)
    {
      std::optional<int32_t> x = calc_val(opcode.arg1, opcode.line, opcode.line_n, label);
      opcode.operand1 = x;
      opcode.operand2 = x.value() >> 8;
    }
    void zero_page(s_opcode& opcode)
    {
      std::optional<int32_t> x = calc_val(opcode.arg1, opcode.line, opcode.line_n, label | zp);
      opcode.operand1 = x;
    }
    void zero_page_relative(s_opcode& opcode)
    {
      std::optional<int32_t> x = calc_val(opcode.arg1, opcode.line, opcode.line_n, label | zp);
      std::optional<int32_t> y = calc_val(opcode.arg2, opcode.line, opcode.line_n, label);
      check_branch(y.value(), y.value() + opcode.size, opcode, 0);
      y = y.value() - opcode.pos - opcode.size;
      opcode.operand1 = x;
      opcode.operand2 = y;
    }
    void immediate_label(s_opcode& opcode)
    {
      std::optional<int32_t> x = calc_val(opcode.arg2, opcode.line, opcode.line_n, label);
      std::string arg1 = opcode.arg1;
      to_lower(arg1);
      if(arg1 == "lo")
        opcode.operand1 = x.value();
      else 
        opcode.operand1 = x.value() >> 8;
    }
    void variables()
    {
      std::vector<s_line>::const_iterator it = in_vector.begin();
      for(;it != in_vector.end(); it++)
      {
        if(std::regex_match(it->line, m, rx_label))
        {
          add_to_map(it->line, it->line_n, 1);
        }
      }
      for(it = in_vector.begin(); it != in_vector.end(); it++)
      {
        if(std::regex_match(it->line, m, rx_var))
        {
          add_to_map(it->line, it->line_n, 0);
        }
      }
    }
    void add_to_ss(const uint32_t& line_n, const std::string msg)
    {
      error = 1;
      ss << "[" + std::to_string(line_n) + "] " + msg + '\n';
    }
    void add_to_map(const std::string& line, const uint32_t& line_n, bool i)
    {
      std::string arg0 = m[1].str();
      std::string arg1 = m[2].str();
      arg1 = std::regex_replace(arg1, rx_space, "");
      if((arg0.length() == 1) && (arg0[0] | 0x20) == 'a')
      {
        add_to_ss(line_n, line + " [aA] is reserved for accumulator addressing mode");
        return;
      }
      s_var var{arg1, line, std::nullopt, line_n};
      std::optional<int32_t> x;
      if(var_map.find(arg0) != var_map.end() || label_map.find(arg0) != label_map.end())
      {
        add_to_ss(line_n, line + " (already exists)");
        return;
      }
      if(arg1[0] < '0' || arg1[0] > '9')
      {
        std::unordered_map<std::string, s_var>::const_iterator it = var_map.find(arg1);
        if(it != var_map.end())
        {
          x = it->second.val;
          var.val = x;
          var_map.emplace(arg0, std::move(var));
          return;
        }
      }
      if(!i)
      {
        x = calc_val(arg1, line, line_n, label);
      }
      else 
      {
        label_map.emplace(arg0, s_label{line_n,std::nullopt});
        return;
      }
      var.val = x;
      var_map.emplace(arg0, std::move(var));
    }
    int get_base(const std::string& str, const std::string& line, const uint32_t& line_n)
    {
      int x{};
      try
      {
        if(str[0] == '$')
        {
          if(std::regex_search(&str[1], rx_hex))
          {
            add_to_ss(line_n, line + " (not hexadecimal)");
            return 0;
          }
          x = std::stol(&str[1], 0, 16);
        }
        else if(str[0] == '%')
        {
          if(std::regex_search(&str[1], rx_binary))
          {
            add_to_ss(line_n, line + " (not binary)");
            return 0;
          }
          x = std::stol(&str[1], 0, 2);
        }
        else 
        {
          if(std::regex_search(str, rx_decimal))
          {
            add_to_ss(line_n, line + " (variable does not exist or value is not decimal)");
            return 0;
          }
          x = std::stol(str);
        }
      }
      catch(const std::invalid_argument& e)
      {
        add_to_ss(line_n, line + " (invalid argument)");
        return 0;
      }
      catch(const std::out_of_range& e)
      {
        add_to_ss(line_n, line + " (value out of range)");
        return 0;
      }
      return x;
    }
    void to_lower(std::string& str)
    {
      for(size_t i{}; i < str.size(); i++)
      {
        str[i] = std::tolower(str[i]);
      }
    }
  public:
    asm_65c02(int argc, char **argv) : argc{argc}, argv{argv}
    {
      read_file(argv[1]);
      if(in_vector.size() < 1)
        throw std::invalid_argument("empty file");
      variables();
      loop();
      labels();
      if(error)
        throw std::invalid_argument(ss.str());
      write();
    }
};
int main(int argc, char** argv)
{
  if(argc < 3)
  {
    throw std::invalid_argument("arg1[in] arg2[out]");
  }
  asm_65c02 asm_6502(argc, argv);
  return 0;
}
