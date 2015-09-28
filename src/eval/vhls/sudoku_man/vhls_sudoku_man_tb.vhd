library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity vhls_sim_tb is
end vhls_sim_tb;

architecture sim of vhls_sim_tb is

    component vhls_sudoku_man is
    port (
    ap_clk : IN STD_LOGIC;
    ap_rst : IN STD_LOGIC;
    ap_start : IN STD_LOGIC;
    ap_done : OUT STD_LOGIC;
    ap_idle : OUT STD_LOGIC;
    ap_ready : OUT STD_LOGIC;
    puzzle_address0 : OUT STD_LOGIC_VECTOR (6 downto 0);
    puzzle_ce0 : OUT STD_LOGIC;
    puzzle_we0 : OUT STD_LOGIC;
    puzzle_d0 : OUT STD_LOGIC_VECTOR (31 downto 0);
    puzzle_q0 : IN STD_LOGIC_VECTOR (31 downto 0);
    ap_return : OUT STD_LOGIC_VECTOR (0 downto 0)
    );
    end component;

    signal ap_clk :  STD_LOGIC := '0';
    signal ap_rst :  STD_LOGIC := '1';
    signal ap_start :  STD_LOGIC := '0';
    signal ap_done :  STD_LOGIC;
    signal ap_idle :  STD_LOGIC;
    signal ap_ready :  STD_LOGIC;
    signal puzzle_address0 : STD_LOGIC_VECTOR (6 downto 0);
    signal puzzle_ce0 : STD_LOGIC;
    signal puzzle_we0 : STD_LOGIC;
    signal puzzle_d0 : STD_LOGIC_VECTOR (31 downto 0);
    signal puzzle_q0 : STD_LOGIC_VECTOR (31 downto 0);
    signal ap_return : STD_LOGIC_VECTOR (0 downto 0);

    signal cycles : unsigned(31 downto 0) := (others=>'0');
    signal start_cycles : unsigned(31 downto 0) := (others=>'0');
    signal total_cycles : unsigned(31 downto 0) := (others=>'0');

    type array_t is array(0 to 80) of integer;
    shared variable array_stg : array_t;
begin

    process begin
        wait for 10ns;
        ap_clk <= not ap_clk;
        wait for 10ns;
        ap_clk <= not ap_clk;
        cycles <= cycles+1;
    end process;

    dut : vhls_sudoku_man port map(
        ap_clk => ap_clk,
        ap_rst => ap_rst,
        ap_start => ap_start,
        ap_done => ap_done,
        ap_idle => ap_idle,
        ap_ready => ap_ready,
        puzzle_address0 => puzzle_address0,
        puzzle_ce0 => puzzle_ce0,
        puzzle_we0 => puzzle_we0,
        puzzle_d0 => puzzle_d0,
        puzzle_q0 => puzzle_q0,
        ap_return => ap_return
    );

    process(ap_clk) begin
        if (rising_edge(ap_clk)) then
            if puzzle_ce0='1' then
                -- VHLS-HACK : Vivado HLS will generate speculative reads out of bounds, AFAICT.
                -- Seems a bit dodgy to me...
                if puzzle_we0='1' then
                    assert false report "Write "&integer'image(to_integer(unsigned(puzzle_address0)))&", cycles="&integer'image(to_integer(cycles)) severity note;
                    if to_integer(unsigned(puzzle_address0)) < 81 then
                        array_stg(to_integer(unsigned(puzzle_address0))) := to_integer(unsigned(puzzle_d0));
                    end if;
                else
                    assert false report "Read "&integer'image(to_integer(unsigned(puzzle_address0)))&", cycles="&integer'image(to_integer(cycles)) severity note;
                    if to_integer(unsigned(puzzle_address0)) < 81 then
                        puzzle_q0 <= std_logic_vector(to_unsigned(array_stg(to_integer(unsigned(puzzle_address0))),32));
                    end if;
                end if;
            end if;
        end if;
    end process;

    process
        variable r,c:integer;
        variable got,ref:integer;
    begin
        array_stg := (
        0,0,3, 0,2,0, 6,0,0,
        9,0,0, 3,0,5, 0,0,1,
        0,0,1, 8,0,6, 4,0,0,
        0,0,8, 1,0,2, 9,0,0,
        7,0,0, 0,0,0, 0,0,8,
        0,0,6, 7,0,8, 2,0,0,
        0,0,2, 6,0,9, 5,0,0,
        8,0,0, 2,0,3, 0,0,9,
        0,0,5, 0,1,0, 3,0,0
        );

        wait until rising_edge(ap_clk);
        ap_rst<='0';

        while ap_idle ='0' loop
            wait until rising_edge(ap_clk);
        end loop;

        ap_start <= '1';
        start_cycles <= cycles;

        while ap_done ='0' loop
            wait until rising_edge(ap_clk);
            ap_start <= '0';
        end loop;

        total_cycles <= cycles-start_cycles;
        wait until rising_edge(ap_clk);

        assert false report "DataPoint: n=81, cycles="&integer'image(to_integer(total_cycles)) severity note;

       assert false report "Ok : Simulation Finished, cycles="&integer'image(to_integer(cycles)) severity failure;
    end process;
end sim;
