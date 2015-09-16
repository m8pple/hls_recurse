library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity vhls_sim_tb is
end vhls_sim_tb;

architecture sim of vhls_sim_tb is

    component vhls_ackerman is
    port (
        ap_clk : IN STD_LOGIC;
        ap_rst : IN STD_LOGIC;
        ap_start : IN STD_LOGIC;
        ap_done : OUT STD_LOGIC;
        ap_idle : OUT STD_LOGIC;
        ap_ready : OUT STD_LOGIC;
        m : IN STD_LOGIC_VECTOR (31 downto 0);
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
    signal m :  STD_LOGIC_VECTOR (31 downto 0);
    signal ap_return :  STD_LOGIC_VECTOR (31 downto 0);

    signal result : unsigned(31 downto 0);

    signal cycles : unsigned(31 downto 0) := (others=>'0');
begin

    process begin
        wait for 10ns;
        ap_clk <= not ap_clk;
        wait for 10ns;
        ap_clk <= not ap_clk;
        cycles <= cycles+1;
    end process;

    dut : vhls_ackerman port map(
        ap_clk => ap_clk,
        ap_rst => ap_rst,
        ap_start => ap_start,
        ap_done => ap_done,
        ap_idle => ap_idle,
        ap_ready => ap_ready,
        n => n,
        m => m,
        ap_return=>ap_return
    );

    process begin
        wait until rising_edge(ap_clk);
        ap_rst<='0';

        while ap_idle ='0' loop
            wait until rising_edge(ap_clk);
        end loop;

        m <= std_logic_vector(to_unsigned(3,32));
        n <= std_logic_vector(to_unsigned(4,32));
        ap_start <= '1';

        while ap_done ='0' loop
            wait until rising_edge(ap_clk);
            ap_start <= '0';
        end loop;

        result <= unsigned(ap_return);
        wait until rising_edge(ap_clk);

        assert result=to_unsigned(125,32) report "FAIL : Incorrect answer, got "&(integer'image(to_integer(result)))&", expected 125" severity failure;

        assert false report "Ok : Simulation Finished, cycles="&integer'image(to_integer(cycles)) severity failure;
    end process;
end sim;
