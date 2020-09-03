import java.sql.*;

public class BrandDB {
    private Connection conn;
    private String brand;
    private String eventsTable;
    private String carsTable;

    public BrandDB(String brand){
        this.brand = brand.toLowerCase();
        this.conn = null;
        this.eventsTable = "CREATE TABLE " + this.brand + "Events(" +
                          "id              int             NOT NULL," +
                          "alert           boolean         NOT NULL," +
                          "prio            boolean         NOT NULL," +
                          "dest            varchar(8)," +
                          "init            timestamp       NOT NULL," +
                          "ttl             int," +
                          "coordX          int," +
                          "coordY          int," +
                          "info            varchar(150)    NOT NULL," +
                          "PRIMARY KEY(id, alert, dest, init));";
        this.carsTable =  "CREATE TABLE " + this.brand + "Cars(" +
                          "registration    varchar(8)      NOT NULL," +
                          "model           varchar(45)," +
                          "owner           varchar(45)     NOT NULL," +
                          "date            date            NOT NULL," +
                          "PRIMARY KEY (registration));";
    }

    private void resetEvents() throws SQLException {
        Statement st = conn.createStatement();
        st.executeUpdate("DROP TABLE IF EXISTS " + brand + "Events;");
        st.executeUpdate(eventsTable);
    }

    private void resetCars() throws SQLException {
        Statement st = conn.createStatement();
        st.executeUpdate("DROP TABLE IF EXISTS " + brand + "Cars;");
        st.executeUpdate(carsTable);
    }

    public void connect(){
        try {
            Class.forName("org.sqlite.JDBC");
            this.conn = DriverManager.getConnection("jdbc:sqlite:" + brand + ".db");
            resetEvents();
            resetCars();
        }catch (ClassNotFoundException | SQLException e) {
            e.printStackTrace();
        }
    }

    public synchronized void insertEvent(int id, boolean alert, boolean prio, String dest, long init, int ttl, int coordX, int coordY, String info){
        try {
            PreparedStatement st = conn.prepareStatement("INSERT INTO " +
                                                         brand + "Events(id, alert, prio, dest, init, ttl, coordX, coordY, info) " +
                                                         "VALUES (?,?,?,?,?,?,?,?,?);");
            st.setInt(1, id);
            st.setBoolean(2, alert);
            st.setBoolean(3, prio);
            st.setString(4, dest);
            st.setTimestamp(5, new Timestamp(init));
            st.setInt(6, ttl);
            st.setInt(7, coordX);
            st.setInt(8, coordY);
            st.setString(9, info);
            st.execute();
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public void insertCar(String registration, String[] modelNowner, String date){
        try {
            Statement st = conn.createStatement();
            st.execute("INSERT INTO " + brand + "Cars " +
                       "VALUES ('" + registration + "','" + modelNowner[0] + "','" + modelNowner[1].trim() + "','" + date + "');");
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public void printAll(){
        try {
            Statement st = conn.createStatement();
            ResultSet rs=st.executeQuery("SELECT * FROM " + brand + "Events;");
            System.out.println("+------+-------+-------+--------+----------+-------+-------+-------+--------------------------------+");
            System.out.println("|  ID  | Alert | Prio  |  Dest  |   Init   |  TTL  |   X   |   Y   |              INFO              |");
            System.out.println("+------+-------+-------+--------+----------+-------+-------+-------+--------------------------------+");
            while(rs.next()){
                System.out.format("| %-4d | %-5b | %-5b | %-6s | %s | %-5d | %-5d | %-5d | %-30s |%n", rs.getInt(1),     rs.getBoolean(2),
                                                                                                       rs.getBoolean(3), rs.getString(4),
                                                                                                       rs.getTime(5),    rs.getInt(6),
                                                                                                       rs.getInt(7),     rs.getInt(8),
                                                                                                       rs.getString(9));
            }
            System.out.println("+------+-------+-------+--------+----------+-------+-------+-------+--------------------------------+");
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public void printAlerts(){
        try {
            Statement st = conn.createStatement();
            ResultSet rs = st.executeQuery("SELECT * FROM " + brand + "Events WHERE alert = true;");
            System.out.println("+------+-------+----------+-------+-------+-------+--------------+");
            System.out.println("|  ID  | Prio  |   Init   |  TTL  |   X   |   Y   |     Info     |");
            System.out.println("+------+-------+----------+-------+-------+-------+--------------+");
            while(rs.next()) {
                System.out.format("| %-4d | %-5b | %s | %-5d | %-5d | %-5d | %-12s |%n", rs.getInt(1),  rs.getBoolean(2),
                                                                                       rs.getTime(5), rs.getInt(6),
                                                                                       rs.getInt(7),  rs.getInt(8),
                                                                                       rs.getString(9));
            }
            System.out.println("+------+-------+----------+-------+-------+-------+--------------+");
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public void printServices(){
        try {
            Statement st = conn.createStatement();
            ResultSet rs = st.executeQuery( "SELECT * FROM " + brand + "Events WHERE alert = false;");
            System.out.println("+------+--------+----------+--------------------------------+");
            System.out.println("|  ID  |  Dest  |   Init   |              INFO              |");
            System.out.println("+------+--------+----------+--------------------------------+");
            while(rs.next()) {
                System.out.format("| %-4d | %s | %s | %-30s |%n", rs.getInt(1),  rs.getString(4),
                                                                                rs.getTime(5), rs.getString(9));
            }
            System.out.println("+------+--------+----------+--------------------------------+");
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public void printEvent(Integer id, boolean alert){
        try {
            Statement st = conn.createStatement();
            ResultSet rs = st.executeQuery( "SELECT * FROM " + brand + "Events " +
                                            "WHERE id = " + id + " AND alert = " + alert + ";");
            if (alert) {
                System.out.println("+------+-------+----------+-------+-------+-------+--------------+");
                System.out.println("|  ID  | Prio  |   Init   |  TTL  |   X   |   Y   |     Info     |");
                System.out.println("+------+-------+----------+-------+-------+-------+--------------+");
                while(rs.next()){
                    System.out.format("| %-4d | %-5b | %s | %-5d | %-5d | %-5d | %-12s |%n", rs.getInt(1),  rs.getBoolean(2),
                                                                                           rs.getTime(5), rs.getInt(6),
                                                                                           rs.getInt(7),  rs.getInt(8),
                                                                                           rs.getString(9));
                }
                System.out.println("+------+-------+----------+-------+-------+-------+--------------+");
            } else {
                System.out.println("+------+--------+----------+--------------------------------+");
                System.out.println("|  ID  |  Dest  |   Init   |              INFO              |");
                System.out.println("+------+--------+----------+--------------------------------+");
                while(rs.next()) {
                    System.out.format("| %-4d | %s | %s | %-5d | %-5d | %-30s |%n", rs.getInt(1),  rs.getString(4),
                                                                                    rs.getTime(5), rs.getString(9));
                }
                System.out.println("+------+--------+----------+--------------------------------+");
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public void printCars(){
        try {
            Statement st = conn.createStatement();
            ResultSet rs = st.executeQuery("SELECT * FROM " + brand + "Cars;");
            System.out.println("+--------------+------------------------------+----------------+------------+");
            System.out.println("| Registration |             Model            |     Owner      |    Date    |");
            System.out.println("+--------------+------------------------------+----------------+------------+");
            while (rs.next()) {
                System.out.format("|    %-6s    | %-28s | %-14s | %-10s |%n", rs.getString(1), rs.getString(2),
                                                                              rs.getString(3), rs.getString(4));
            }
            System.out.println("+--------------+------------------------------+----------------+------------+");
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
    public void printCar(String registration){
        try {
            Statement st = conn.createStatement();
            ResultSet rs = st.executeQuery("SELECT * FROM " + brand + "Cars WHERE registration = '" + registration + "';");
            System.out.println("+--------------+------------------------------+----------------+------------+");
            System.out.println("| Registration |             Model            |     Owner      |    Date    |");
            System.out.println("+--------------+------------------------------+----------------+------------+");
            while (rs.next()) {
                System.out.format("|    %-6s    | %-28s | %-14s | %-10s |%n", rs.getString(1), rs.getString(2),
                                                                              rs.getString(3), rs.getString(4));
            }
            System.out.println("+--------------+------------------------------+----------------+------------+");
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
}
