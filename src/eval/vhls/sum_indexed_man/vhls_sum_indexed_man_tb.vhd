library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity vhls_sim_tb is
end vhls_sim_tb;

architecture sim of vhls_sim_tb is

    component vhls_sum_indexed_man is
    port (
    ap_clk : IN STD_LOGIC;
    ap_rst : IN STD_LOGIC;
    ap_start : IN STD_LOGIC;
    ap_done : OUT STD_LOGIC;
    ap_idle : OUT STD_LOGIC;
    ap_ready : OUT STD_LOGIC;
    n : IN STD_LOGIC_VECTOR (31 downto 0);
    array_r_address0 : OUT STD_LOGIC_VECTOR (11 downto 0);
    array_r_ce0 : OUT STD_LOGIC;
    array_r_we0 : OUT STD_LOGIC;
    array_r_d0 : OUT STD_LOGIC_VECTOR (31 downto 0);
    array_r_q0 : IN STD_LOGIC_VECTOR (31 downto 0);
    array_r_address1 : OUT STD_LOGIC_VECTOR (11 downto 0);
    array_r_ce1 : OUT STD_LOGIC;
    array_r_q1 : IN STD_LOGIC_VECTOR (31 downto 0)
    );
    end component;

    signal ap_clk :  STD_LOGIC := '0';
    signal ap_rst :  STD_LOGIC := '1';
    signal ap_start :  STD_LOGIC := '0';
    signal ap_done :  STD_LOGIC;
    signal ap_idle :  STD_LOGIC;
    signal ap_ready :  STD_LOGIC;
    signal n :  STD_LOGIC_VECTOR (31 downto 0);
    signal array_r_address0 :  STD_LOGIC_VECTOR (11 downto 0);
    signal array_r_ce0 :  STD_LOGIC;
    signal array_r_we0 :  STD_LOGIC;
    signal array_r_d0 :  STD_LOGIC_VECTOR (31 downto 0);
    signal array_r_q0 :  STD_LOGIC_VECTOR (31 downto 0);
    signal array_r_address1 :  STD_LOGIC_VECTOR (11 downto 0);
    signal array_r_ce1 :  STD_LOGIC;
    signal array_r_q1 :  STD_LOGIC_VECTOR (31 downto 0);

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

    dut : vhls_sum_indexed_man port map(
        ap_clk => ap_clk,
        ap_rst => ap_rst,
        ap_start => ap_start,
        ap_done => ap_done,
        ap_idle => ap_idle,
        ap_ready => ap_ready,
        n => n,
        array_r_address0 => array_r_address0 ,
        array_r_ce0 => array_r_ce0 ,
        array_r_we0 => array_r_we0 ,
        array_r_d0 => array_r_d0 ,
        array_r_q0 => array_r_q0 ,
        array_r_address1 => array_r_address1 ,
        array_r_ce1 => array_r_ce1 ,
        array_r_q1 => array_r_q1
    );

    process(ap_clk) begin
        if (rising_edge(ap_clk)) then
            if array_r_ce1='1' then
                array_r_q1 <= array_stg(to_integer(unsigned(array_r_address1)));
            end if;
        end if;
    end process;
    process(ap_clk) begin
        if (rising_edge(ap_clk)) then
            if array_r_ce0='1' then
                if array_r_we0='1' then
                    array_stg(to_integer(unsigned(array_r_address0))) := array_r_d0;
                else
                    array_r_q0 <= array_stg(to_integer(unsigned(array_r_address0)));
                end if;
            end if;
        end if;
    end process;

    process
        variable curr_n:integer:=2;
        variable tmp:integer;
        variable got,ref:integer;
    begin
        while curr_n <= 4096 loop
            tmp:=0;
            while tmp<curr_n loop
                array_stg(tmp) := std_logic_vector(to_unsigned(tmp, 32));
                tmp:=tmp+1;
            end loop;

            wait until rising_edge(ap_clk);
            ap_rst<='0';

            while ap_idle ='0' loop
                wait until rising_edge(ap_clk);
            end loop;

            n <= std_logic_vector(to_unsigned(curr_n,32));
            ref := curr_n*(curr_n-1)/2;
            ap_start <= '1';
            start_cycles <= cycles;

            while ap_done ='0' loop
                wait until rising_edge(ap_clk);
                ap_start <= '0';
            end loop;

            total_cycles <= cycles-start_cycles;
            wait until rising_edge(ap_clk);
            got := to_integer(unsigned(array_stg(0)));

            assert got=ref report "FAIL : Incorrect answer, got "&(integer'image(got))&", expected "&(integer'image(ref)) severity failure;

            assert false report "DataPoint: n="&integer'image(curr_n)&", cycles="&integer'image(to_integer(total_cycles)) severity note;

            curr_n := curr_n*2;
        end loop;

        assert false report "Ok : Simulation Finished, cycles="&integer'image(to_integer(cycles)) severity failure;
    end process;
end sim;
