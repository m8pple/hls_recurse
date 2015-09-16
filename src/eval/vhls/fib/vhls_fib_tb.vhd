library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity vhls_sim_tb is
end vhls_sim_tb;

architecture sim of vhls_sim_tb is

    component vhls_fib is
    port (
        ap_clk : IN STD_LOGIC;
        ap_rst : IN STD_LOGIC;
        ap_start : IN STD_LOGIC;
        ap_done : OUT STD_LOGIC;
        ap_idle : OUT STD_LOGIC;
        ap_ready : OUT STD_LOGIC;
        n : IN STD_LOGIC_VECTOR (31 downto 0);
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
    signal ap_return :  STD_LOGIC_VECTOR (31 downto 0);

    signal result : unsigned(31 downto 0);

    signal cycles : unsigned(31 downto 0) := (others=>'0');
    signal start_cycles : unsigned(31 downto 0) := (others=>'0');
    signal total_cycles : unsigned(31 downto 0) := (others=>'0');

    type ref_t is array(29 downto 0) of integer;
    signal ref_data : ref_t :=(
        0 => 0, 1 => 1, 2 => 1, 3 => 2, 4 => 3, 5 => 5, 6 => 8, 7 => 13, 8 => 21, 9 => 34,
        10 => 55, 11 => 89, 12=> 144, 13=> 233, 14 => 377, 15 => 610, 16 => 987, 17 => 1597, 18 => 2584, 19 => 4181,
        20 => 6765, 21 => 10946, 22 => 17711, 23 => 28657, 24 => 46368, 25 => 75025, 26 => 121393, 27 => 196418, 28 => 317811, 29 => 514229
    );

begin

    process begin
        wait for 10ns;
        ap_clk <= not ap_clk;
        wait for 10ns;
        ap_clk <= not ap_clk;
        cycles <= cycles+1;
    end process;

    dut : vhls_fib port map(
        ap_clk => ap_clk,
        ap_rst => ap_rst,
        ap_start => ap_start,
        ap_done => ap_done,
        ap_idle => ap_idle,
        ap_ready => ap_ready,
        n => n,
        ap_return=>ap_return
    );

    process
        variable curr_n : integer := 0;
        variable tmp : integer;
        variable got, ref : integer;
    begin
        while curr_n <= 24 loop
            tmp:=0;
            wait until rising_edge(ap_clk);
            ap_rst<='0';

            while ap_idle ='0' loop
                wait until rising_edge(ap_clk);
            end loop;

            n <= std_logic_vector(to_unsigned(curr_n,32));
            ap_start <= '1';
            start_cycles <= cycles;
            ref := ref_data(curr_n);


            while ap_done ='0' loop
                wait until rising_edge(ap_clk);
                ap_start <= '0';
            end loop;

            got := to_integer(unsigned(ap_return));
            total_cycles <= cycles-start_cycles;

            wait until rising_edge(ap_clk);

            assert ref=got report "FAIL : Incorrect answer, got "&(integer'image(got))&", expected "&integer'image(ref) severity failure;

            assert false report "DataPoint: n="&integer'image(curr_n)&", cycles="&integer'image(to_integer(total_cycles)) severity note;

            curr_n := curr_n+1;
        end loop;

        assert false report "Ok : Simulation Finished, cycles="&integer'image(to_integer(cycles)) severity failure;
    end process;
end sim;
