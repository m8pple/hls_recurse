library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity vhls_sim_tb is
end vhls_sim_tb;

architecture sim of vhls_sim_tb is

    component vhls_strassen_indexed_v2 is
    port (
    ap_clk : IN STD_LOGIC;
    ap_rst : IN STD_LOGIC;
    ap_start : IN STD_LOGIC;
    ap_done : OUT STD_LOGIC;
    ap_idle : OUT STD_LOGIC;
    ap_ready : OUT STD_LOGIC;
    p_address0 : OUT STD_LOGIC_VECTOR (22 downto 0);
    p_ce0 : OUT STD_LOGIC;
    p_we0 : OUT STD_LOGIC;
    p_d0 : OUT STD_LOGIC_VECTOR (31 downto 0);
    p_q0 : IN STD_LOGIC_VECTOR (31 downto 0);
    p_address1 : OUT STD_LOGIC_VECTOR (22 downto 0);
    p_ce1 : OUT STD_LOGIC;
    p_we1 : OUT STD_LOGIC;
    p_d1 : OUT STD_LOGIC_VECTOR (31 downto 0);
    p_q1 : IN STD_LOGIC_VECTOR (31 downto 0);
    log2n : IN STD_LOGIC_VECTOR (31 downto 0)
    );
    end component;

    signal ap_clk :  STD_LOGIC := '0';
    signal ap_rst :  STD_LOGIC := '1';
    signal ap_start :  STD_LOGIC := '0';
    signal ap_done :  STD_LOGIC;
    signal ap_idle :  STD_LOGIC;
    signal ap_ready :  STD_LOGIC;
    signal n :  STD_LOGIC_VECTOR (31 downto 0);
    signal p_address0 :  STD_LOGIC_VECTOR (22 downto 0);
    signal p_ce0 :  STD_LOGIC;
    signal p_we0 :  STD_LOGIC;
    signal p_d0 :  STD_LOGIC_VECTOR (31 downto 0);
    signal p_q0 :  STD_LOGIC_VECTOR (31 downto 0);
    signal p_address1 :  STD_LOGIC_VECTOR (22 downto 0);
    signal p_ce1 :  STD_LOGIC;
    signal p_we1 :  STD_LOGIC;
    signal p_d1 :  STD_LOGIC_VECTOR (31 downto 0);
    signal p_q1 :  STD_LOGIC_VECTOR (31 downto 0);
    signal log2n :  STD_LOGIC_VECTOR (31 downto 0);

    signal cycles : unsigned(31 downto 0) := (others=>'0');
    signal start_cycles : unsigned(31 downto 0) := (others=>'0');
    signal total_cycles : unsigned(31 downto 0) := (others=>'0');

    -- 1310720 == 512*512*5
    type array_t is array(1310719 downto 0) of std_logic_vector(31 downto 0);
    shared variable array_stg : array_t;
begin

    process begin
        wait for 10ns;
        ap_clk <= not ap_clk;
        wait for 10ns;
        ap_clk <= not ap_clk;
        cycles <= cycles+1;
    end process;

    dut : vhls_strassen_indexed_v2 port map(
        ap_clk => ap_clk,
        ap_rst => ap_rst,
        ap_start => ap_start,
        ap_done => ap_done,
        ap_idle => ap_idle,
        ap_ready => ap_ready,
        log2n => log2n,
        p_address0 => p_address0,
        p_ce0 => p_ce0,
        p_we0 => p_we0,
        p_d0 => p_d0,
        p_q0 => p_q0,
        p_address1 => p_address1,
        p_ce1 => p_ce1,
        p_we1 => p_we1,
        p_d1 => p_d1,
        p_q1 => p_q1
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
        variable got,ref:integer;
    begin
        while curr_n <= 9 loop

            n:=2**curr_n;
            for r in 0 to n-1 loop
                for c in 0 to n-1 loop
                    array_stg(r*n+c):=std_logic_vector(to_unsigned(r,32));
                    array_stg(n*n+r*n+c):=std_logic_vector(to_unsigned(c,32));
                end loop;
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

            assert false report "DataPoint: n="&integer'image(curr_n)&", cycles="&integer'image(to_integer(total_cycles)) severity note;

            curr_n := curr_n+1;

        end loop;

        assert false report "Ok : Simulation Finished, cycles="&integer'image(to_integer(cycles)) severity failure;
    end process;
end sim;
