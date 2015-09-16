library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity vhls_sim_tb is
end vhls_sim_tb;

architecture sim of vhls_sim_tb is

    component vhls_sort_indexed is
    port (
    ap_clk : IN STD_LOGIC;
    ap_rst : IN STD_LOGIC;
    ap_start : IN STD_LOGIC;
    ap_done : OUT STD_LOGIC;
    ap_idle : OUT STD_LOGIC;
    ap_ready : OUT STD_LOGIC;
    a_address0 : OUT STD_LOGIC_VECTOR (11 downto 0);
    a_ce0 : OUT STD_LOGIC;
    a_we0 : OUT STD_LOGIC;
    a_d0 : OUT STD_LOGIC_VECTOR (31 downto 0);
    a_q0 : IN STD_LOGIC_VECTOR (31 downto 0);
    n : IN STD_LOGIC_VECTOR (31 downto 0)
    );
    end component;

    signal ap_clk :  STD_LOGIC := '0';
    signal ap_rst :  STD_LOGIC := '1';
    signal ap_start :  STD_LOGIC := '0';
    signal ap_done :  STD_LOGIC;
    signal ap_idle :  STD_LOGIC;
    signal ap_ready :  STD_LOGIC;
    signal n :  STD_LOGIC_VECTOR (31 downto 0);
    signal a_address0 :  STD_LOGIC_VECTOR (11 downto 0);
    signal a_ce0 :  STD_LOGIC;
    signal a_we0 :  STD_LOGIC;
    signal a_d0 :  STD_LOGIC_VECTOR (31 downto 0);
    signal a_q0 :  STD_LOGIC_VECTOR (31 downto 0);

    signal cycles : unsigned(31 downto 0) := (others=>'0');
    signal start_cycles : unsigned(31 downto 0) := (others=>'0');
    signal total_cycles : unsigned(31 downto 0) := (others=>'0');
    
    type array_t is array(4095 downto 0) of std_logic_vector(31 downto 0);
    shared variable array_stg : array_t;
begin

    process begin
        wait for 10ns;
        ap_clk <= not ap_clk;
        wait for 10ns;
        ap_clk <= not ap_clk;
        cycles <= cycles+1;
    end process;

    dut : vhls_sort_indexed port map(
        ap_clk => ap_clk,
        ap_rst => ap_rst,
        ap_start => ap_start,
        ap_done => ap_done,
        ap_idle => ap_idle,
        ap_ready => ap_ready,
        n => n,
        a_address0 => a_address0 ,
        a_ce0 => a_ce0 ,
        a_we0 => a_we0 ,
        a_d0 => a_d0 ,
        a_q0 => a_q0
    );

    process(ap_clk) begin
        if (rising_edge(ap_clk)) then
            if a_ce0='1' then
                if a_we0='1' then
                    array_stg(to_integer(unsigned(a_address0))) := a_d0;
                else
                    a_q0 <= array_stg(to_integer(unsigned(a_address0)));
                end if;
            end if;
        end if;
    end process;

    process
        variable curr_n:integer:=4;
        variable tmp:integer;
        variable rng:unsigned(31 downto 0);
        variable rng_wide:unsigned(63 downto 0);
        variable got,ref:integer;
    begin
        while curr_n <= 4096 loop
            tmp:=0;
            rng:=to_unsigned(123456,32);
            while tmp<curr_n loop
                array_stg(tmp) := "001"&std_logic_vector(rng(28 downto 0));
                tmp:=tmp+1;
                rng_wide:=(rng*to_unsigned(1664525,32))+1013904223;
                rng:=rng_wide(31 downto 0);
            end loop;
    
            wait until rising_edge(ap_clk);
            ap_rst<='0';
    
            while ap_idle ='0' loop
                wait until rising_edge(ap_clk);
            end loop;
    
            n <= std_logic_vector(to_unsigned(tmp,32));
            ap_start <= '1';
            start_cycles <= cycles;
    
            while ap_done ='0' loop
                wait until rising_edge(ap_clk);
                ap_start <= '0';
            end loop;
    
            total_cycles <= cycles-start_cycles;
            wait until rising_edge(ap_clk);
    
            tmp:=1;
            while tmp<curr_n loop
                assert unsigned(array_stg(tmp-1))<=unsigned(array_stg(tmp)) report "FAIL : output is not sorted" severity failure;
                tmp:=tmp+1;
            end loop;
            
            assert false report "DataPoint: n="&integer'image(curr_n)&", cycles="&integer'image(to_integer(total_cycles)) severity note;
            
            curr_n := curr_n*2;
            
        end loop;

        assert false report "Ok : Simulation Finished, cycles="&integer'image(to_integer(cycles)) severity failure;
    end process;
end sim;
