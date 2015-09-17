library IEEE;
use IEEE.std_logic_1164.all;
use ieee.numeric_std.all;
library IEEE_PROPOSED;
use ieee_proposed.fixed_float_types.all;
use ieee_proposed.fixed_pkg.all;
use ieee_proposed.float_pkg.all;

entity vhls_sim_tb is
end vhls_sim_tb;

architecture sim of vhls_sim_tb is

    component vhls_fft_indexed is
    port (
    ap_clk : IN STD_LOGIC;
    ap_rst : IN STD_LOGIC;
    ap_start : IN STD_LOGIC;
    ap_done : OUT STD_LOGIC;
    ap_idle : OUT STD_LOGIC;
    ap_ready : OUT STD_LOGIC;
    log2n : IN STD_LOGIC_VECTOR (31 downto 0);
    pIn_re_address0 : OUT STD_LOGIC_VECTOR (11 downto 0);
    pIn_re_ce0 : OUT STD_LOGIC;
    pIn_re_q0 : IN STD_LOGIC_VECTOR (31 downto 0);
    pIn_re_address1 : OUT STD_LOGIC_VECTOR (11 downto 0);
    pIn_re_ce1 : OUT STD_LOGIC;
    pIn_re_q1 : IN STD_LOGIC_VECTOR (31 downto 0);
    pIn_im_address0 : OUT STD_LOGIC_VECTOR (11 downto 0);
    pIn_im_ce0 : OUT STD_LOGIC;
    pIn_im_q0 : IN STD_LOGIC_VECTOR (31 downto 0);
    pIn_im_address1 : OUT STD_LOGIC_VECTOR (11 downto 0);
    pIn_im_ce1 : OUT STD_LOGIC;
    pIn_im_q1 : IN STD_LOGIC_VECTOR (31 downto 0);
    pOut_re_address0 : OUT STD_LOGIC_VECTOR (11 downto 0);
    pOut_re_ce0 : OUT STD_LOGIC;
    pOut_re_we0 : OUT STD_LOGIC;
    pOut_re_d0 : OUT STD_LOGIC_VECTOR (31 downto 0);
    pOut_re_q0 : IN STD_LOGIC_VECTOR (31 downto 0);
    pOut_im_address0 : OUT STD_LOGIC_VECTOR (11 downto 0);
    pOut_im_ce0 : OUT STD_LOGIC;
    pOut_im_we0 : OUT STD_LOGIC;
    pOut_im_d0 : OUT STD_LOGIC_VECTOR (31 downto 0);
    pOut_im_q0 : IN STD_LOGIC_VECTOR (31 downto 0)
    );
    end component;

    signal ap_clk :  STD_LOGIC := '0';
    signal ap_rst :  STD_LOGIC := '1';
    signal ap_start :  STD_LOGIC := '0';
    signal ap_done :  STD_LOGIC;
    signal ap_idle :  STD_LOGIC;
    signal ap_ready :  STD_LOGIC;
    signal log2n : STD_LOGIC_VECTOR (31 downto 0);
    signal pIn_re_address0 :  STD_LOGIC_VECTOR (11 downto 0);
	signal pIn_re_ce0 :  STD_LOGIC;
	signal pIn_re_q0 :  STD_LOGIC_VECTOR (31 downto 0);
	signal pIn_re_address1 :  STD_LOGIC_VECTOR (11 downto 0);
	signal pIn_re_ce1 :  STD_LOGIC;
	signal pIn_re_q1 :  STD_LOGIC_VECTOR (31 downto 0);
	signal pIn_im_address0 :  STD_LOGIC_VECTOR (11 downto 0);
	signal pIn_im_ce0 :  STD_LOGIC;
	signal pIn_im_q0 :  STD_LOGIC_VECTOR (31 downto 0);
	signal pIn_im_address1 :  STD_LOGIC_VECTOR (11 downto 0);
	signal pIn_im_ce1 :  STD_LOGIC;
	signal pIn_im_q1 :  STD_LOGIC_VECTOR (31 downto 0);
	signal pOut_re_address0 :  STD_LOGIC_VECTOR (11 downto 0);
	signal pOut_re_ce0 :  STD_LOGIC;
	signal pOut_re_we0 :  STD_LOGIC;
	signal pOut_re_d0 :  STD_LOGIC_VECTOR (31 downto 0);
	signal pOut_re_q0 :  STD_LOGIC_VECTOR (31 downto 0);
	signal pOut_im_address0 :  STD_LOGIC_VECTOR (11 downto 0);
	signal pOut_im_ce0 :  STD_LOGIC;
	signal pOut_im_we0 :  STD_LOGIC;
	signal pOut_im_d0 :  STD_LOGIC_VECTOR (31 downto 0);
	signal pOut_im_q0 :  STD_LOGIC_VECTOR (31 downto 0);

    signal cycles : unsigned(31 downto 0) := (others=>'0');
    signal start_cycles : unsigned(31 downto 0) := (others=>'0');
    signal total_cycles : unsigned(31 downto 0) := (others=>'0');

    type array_t is array(4095 downto 0) of std_logic_vector(31 downto 0);

    shared variable pIn_re_stg, pIn_im_stg, pOut_im_stg, pOut_re_stg : array_t;
begin

    process begin
        wait for 10ns;
        ap_clk <= not ap_clk;
        wait for 10ns;
        ap_clk <= not ap_clk;
        cycles <= cycles+1;
    end process;

    dut : vhls_fft_indexed port map(
        ap_clk => ap_clk,
        ap_rst => ap_rst,
        ap_start => ap_start,
        ap_done => ap_done,
        ap_idle => ap_idle,
        ap_ready => ap_ready,
        log2n => log2n,
        pIn_re_address0 => pIn_re_address0,
        pIn_re_ce0 => pIn_re_ce0,
        pIn_re_q0 => pIn_re_q0,
        pIn_re_address1 => pIn_re_address1,
        pIn_re_ce1 => pIn_re_ce1,
        pIn_re_q1 => pIn_re_q1,
        pIn_im_address0 => pIn_im_address0,
        pIn_im_ce0 => pIn_im_ce0,
        pIn_im_q0 => pIn_im_q0,
        pIn_im_address1 => pIn_im_address1,
        pIn_im_ce1 => pIn_im_ce1,
        pIn_im_q1 => pIn_im_q1,
        pOut_re_address0 => pOut_re_address0,
        pOut_re_ce0 => pOut_re_ce0,
        pOut_re_we0 => pOut_re_we0,
        pOut_re_d0 => pOut_re_d0,
        pOut_re_q0 => pOut_re_q0,
        pOut_im_address0 => pOut_im_address0,
        pOut_im_ce0 => pOut_im_ce0,
        pOut_im_we0 => pOut_im_we0,
        pOut_im_d0 => pOut_im_d0,
        pOut_im_q0 => pOut_im_q0
    );

    process(ap_clk) begin
        if (rising_edge(ap_clk)) then
            if pIn_re_ce0='1' then
                pIn_re_q0 <= pIn_re_stg(to_integer(unsigned(pIn_re_address0)));
            end if;
        end if;
    end process;
    process(ap_clk) begin
        if (rising_edge(ap_clk)) then
            if pIn_re_ce1='1' then
                pIn_re_q1 <= pIn_re_stg(to_integer(unsigned(pIn_re_address1)));
            end if;
        end if;
    end process;
    process(ap_clk) begin
        if (rising_edge(ap_clk)) then
            if pIn_im_ce0='1' then
                pIn_im_q0 <= pIn_im_stg(to_integer(unsigned(pIn_im_address0)));
            end if;
        end if;
    end process;
    process(ap_clk) begin
        if (rising_edge(ap_clk)) then
            if pIn_im_ce1='1' then
                pIn_im_q1 <= pIn_im_stg(to_integer(unsigned(pIn_im_address1)));
            end if;
        end if;
    end process;
    
    process(ap_clk) begin
        if (rising_edge(ap_clk)) then
            if pOut_re_ce0='1' then
                if pOut_re_we0='1' then
                    pOut_re_stg(to_integer(unsigned(pOut_re_address0))) := pOut_re_d0;
                else
                    pOut_re_q0 <= pOut_re_stg(to_integer(unsigned(pOut_re_address0)));
                end if;
            end if;
        end if;
    end process;
    process(ap_clk) begin
        if (rising_edge(ap_clk)) then
            if pOut_im_ce0='1' then
                if pOut_im_we0='1' then
                    pOut_im_stg(to_integer(unsigned(pOut_im_address0))) := pOut_im_d0;
                else
                    pOut_im_q0 <= pOut_im_stg(to_integer(unsigned(pOut_im_address0)));
                end if;
            end if;
        end if;
    end process;

     process
         variable curr_n:integer:=2;
         variable tmp, i, n:integer;
         variable got,ref,err:float32;
         
         variable zero : float32 := (others=>'0');
     begin
        
         while curr_n <= 12 loop
             n:=2**curr_n;
             tmp:=0;
             while tmp<n loop
                 pIn_re_stg(tmp) := std_logic_vector(to_float(tmp, zero));
                 pIn_im_stg(tmp) := std_logic_vector(zero);
                 pOut_re_stg(tmp) := (others=>'0');
                 pOut_im_stg(tmp) := (others=>'0');
                 tmp:=tmp+1;
             end loop;

             wait until rising_edge(ap_clk);
             ap_rst<='0';

             while ap_idle ='0' loop
                 wait until rising_edge(ap_clk);
             end loop;

             log2n <= std_logic_vector(to_unsigned(curr_n,32));
             ap_start <= '1';
             start_cycles <= cycles;

             while ap_done ='0' loop
                 wait until rising_edge(ap_clk);
                 ap_start <= '0';
             end loop;

             total_cycles <= cycles-start_cycles;
             wait until rising_edge(ap_clk);

             
             i:=0;
             while i < n loop
                if i=0 then
                    ref := to_float( (2** (curr_n-1)) * (n-1), zero);
                else
                    ref := to_float(-(2**(curr_n-1)), zero);
                end if;
                got := to_float(pOut_re_stg(i), zero);
                err := abs((ref-got)/ref);
             
                if err > to_float(0.001,zero) then
                    assert false report "FAIL : output is not correct" severity failure;
                end if;
                i:=i+1;
             end loop;
             
             assert false report "DataPoint: n="&integer'image(n)&", cycles="&integer'image(to_integer(total_cycles)) severity note;

             curr_n := curr_n+1;

         end loop;

         assert false report "Ok : Simulation Finished, cycles="&integer'image(to_integer(cycles)) severity failure;
     end process;
end sim;
