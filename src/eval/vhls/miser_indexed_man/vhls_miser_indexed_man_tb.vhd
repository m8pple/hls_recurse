library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
library IEEE_PROPOSED;
use ieee_proposed.fixed_float_types.all;
use ieee_proposed.fixed_pkg.all;
use ieee_proposed.float_pkg.all;

entity vhls_sim_tb is
end vhls_sim_tb;

architecture sim of vhls_sim_tb is

    component vhls_miser_indexed_man is
    port (
    ap_clk : IN STD_LOGIC;
    ap_rst : IN STD_LOGIC;
    ap_start : IN STD_LOGIC;
    ap_done : OUT STD_LOGIC;
    ap_idle : OUT STD_LOGIC;
    ap_ready : OUT STD_LOGIC;
    p_address0 : OUT STD_LOGIC_VECTOR (11 downto 0);
    p_ce0 : OUT STD_LOGIC;
    p_we0 : OUT STD_LOGIC;
    p_d0 : OUT STD_LOGIC_VECTOR (31 downto 0);
    p_q0 : IN STD_LOGIC_VECTOR (31 downto 0);
    p_address1 : OUT STD_LOGIC_VECTOR (11 downto 0);
    p_ce1 : OUT STD_LOGIC;
    --p_we1 : OUT STD_LOGIC;
    p_q1 : IN STD_LOGIC_VECTOR (31 downto 0);
    --p_d1 : OUT STD_LOGIC_VECTOR (31 downto 0);
    regn : IN STD_LOGIC_VECTOR (31 downto 0);
    npts : IN STD_LOGIC_VECTOR (31 downto 0);
    freeStart : IN STD_LOGIC_VECTOR (31 downto 0);
    ap_return : OUT STD_LOGIC_VECTOR (31 downto 0)
    );
    end component;

    signal ap_clk :  STD_LOGIC := '0';
    signal ap_rst :  STD_LOGIC := '1';
    signal ap_start :  STD_LOGIC := '0';
    signal ap_done :  STD_LOGIC;
    signal ap_idle :  STD_LOGIC;
    signal ap_ready :  STD_LOGIC;
    signal n :  STD_LOGIC_VECTOR (31 downto 0);
    signal p_address0 :  STD_LOGIC_VECTOR (11 downto 0);
    signal p_ce0 :  STD_LOGIC;
    signal p_we0 :  STD_LOGIC;
    signal p_d0 :  STD_LOGIC_VECTOR (31 downto 0);
    signal p_q0 :  STD_LOGIC_VECTOR (31 downto 0);
    signal p_address1 :  STD_LOGIC_VECTOR (11 downto 0);
    signal p_ce1 :  STD_LOGIC;
    signal p_we1 :  STD_LOGIC :='0';
    signal p_d1 :  STD_LOGIC_VECTOR (31 downto 0);
    signal p_q1 :  STD_LOGIC_VECTOR (31 downto 0);
    signal regn : STD_LOGIC_VECTOR (31 downto 0);
    signal npts : STD_LOGIC_VECTOR (31 downto 0);
    signal dith : STD_LOGIC_VECTOR (31 downto 0);
    signal freeStart : STD_LOGIC_VECTOR (31 downto 0);
    signal ap_return : STD_LOGIC_VECTOR (31 downto 0) ;

    signal cycles : unsigned(31 downto 0) := (others=>'0');
    signal start_cycles : unsigned(31 downto 0) := (others=>'0');
    signal total_cycles : unsigned(31 downto 0) := (others=>'0');

    -- 81920 == 128*128*5
    type array_t is array(81919 downto 0) of std_logic_vector(31 downto 0);
    shared variable array_stg : array_t;
begin

    process begin
        wait for 10ns;
        ap_clk <= not ap_clk;
        wait for 10ns;
        ap_clk <= not ap_clk;
        cycles <= cycles+1;
    end process;

    dut : vhls_miser_indexed_man port map(
        ap_clk => ap_clk,
        ap_rst => ap_rst,
        ap_start => ap_start,
        ap_done => ap_done,
        ap_idle => ap_idle,
        ap_ready => ap_ready,
        p_address0 => p_address0,
        p_ce0 => p_ce0,
        p_we0 => p_we0,
        p_d0 => p_d0,
        p_q0 => p_q0,
        p_address1 => p_address1,
        p_ce1 => p_ce1,
        --p_we1 => p_we1,
        p_q1 => p_q1,
        --p_d1 => p_d1,
        regn => regn,
        npts => npts,
        freeStart => freeStart,
        ap_return => ap_return
    );

    process(ap_clk) begin
        if (rising_edge(ap_clk)) then
            if p_ce0='1' then
                if p_we0='1' then
                    array_stg(to_integer(unsigned(p_address0))) := p_d0;
                else
                    p_q0 <= array_stg(to_integer(unsigned(p_address0)));
                end if;
            end if;
        end if;
    end process;
    process(ap_clk) begin
        if (rising_edge(ap_clk)) then
            if p_ce1='1' then
                if p_we1='1' then
                    array_stg(to_integer(unsigned(p_address1))) := p_d1;
                else
                    p_q1 <= array_stg(to_integer(unsigned(p_address1)));
                end if;
            end if;
        end if;
    end process;

    process
        variable curr_n:integer:=1;
        variable n:integer;
        variable r,c:integer;
        variable got,ref:float32;

        variable zero : float32 := (others=>'0');
    begin
        while curr_n <= 65536 loop

            array_stg(0) := std_logic_vector(zero);
            array_stg(1) := std_logic_vector(zero);
            array_stg(2) := std_logic_vector(zero);
            array_stg(3) := std_logic_vector(zero);
            array_stg(4) := std_logic_vector(to_float(1, zero));
            array_stg(5) := std_logic_vector(to_float(2, zero));
            array_stg(6) := std_logic_vector(to_float(3, zero));
            array_stg(7) := std_logic_vector(to_float(4, zero));

            wait until rising_edge(ap_clk);
            ap_rst<='0';

            while ap_idle ='0' loop
                wait until rising_edge(ap_clk);
            end loop;

            regn <= std_logic_vector(to_unsigned(0,32));
            npts <= std_logic_vector(to_unsigned(curr_n,32));
            freeStart <= std_logic_vector(to_unsigned(8,32));
            ap_start <= '1';
            start_cycles <= cycles;

            while ap_done ='0' loop
                wait until rising_edge(ap_clk);
                ap_start <= '0';
            end loop;

            got := to_float(ap_return,zero);
            total_cycles <= cycles-start_cycles;
            wait until rising_edge(ap_clk);

            assert false report "DataPoint: n="&integer'image(curr_n)&", cycles="&integer'image(to_integer(total_cycles))&", value="&real'image(to_real(got)) severity note;

            curr_n := curr_n*2;

        end loop;

        assert false report "Ok : Simulation Finished, cycles="&integer'image(to_integer(cycles)) severity failure;
    end process;
end sim;
