//import java.sql.*;

import java.security.acl.Owner;
import java.sql.*;
import java.sql.Statement;

public class IMTTDB {
    private Connection conn;
    private String alertTable;
    private String carsTable;

    public IMTTDB(){
        this.conn = null;
        this.alertTable = "CREATE TABLE imttAlerts(" +
                          "id              int             NOT NULL," +
                          "prio            boolean         NOT NULL," +
                          "init            timestamp       NOT NULL," +
                          "ttl             int             NOT NULL," +
                          "coordX          int," +
                          "coordY          int," +
                          "info            varchar(150)    NOT NULL," +
                          "PRIMARY KEY (id));";
        this.carsTable =  "CREATE TABLE imttCars(" +
                          "registration    varchar(8)      NOT NULL," +
                          "brand           varchar(6)      NOT NULL," +
                          "model           varchar(45)," +
                          "owner           varchar(45)     NOT NULL," +
                          "date            date            NOT NULL," +
                          "PRIMARY KEY (registration));";
    }

    private void resetAlert() throws SQLException {
        Statement st = conn.createStatement();
        st.executeUpdate("DROP TABLE IF EXISTS imttAlerts;");
        st.executeUpdate(alertTable);
    }

    private void resetCars() throws SQLException {
        Statement st = conn.createStatement();
        st.executeUpdate("DROP TABLE IF EXISTS imttCars;");
        st.executeUpdate(carsTable);
    }

    public void connect(){
        try {
            Class.forName("org.sqlite.JDBC");
            this.conn = DriverManager.getConnection("jdbc:sqlite:imtt.db");
            resetAlert();
            resetCars();
        }catch (ClassNotFoundException | SQLException e) {
            e.printStackTrace();
        }
    }

    public void insertAlert(int id, boolean prio, long init, int ttl, int coordX, int coordY, String info){
        try {
            PreparedStatement st = conn.prepareStatement("INSERT INTO " +
                                                         "imttAlerts(id, prio, init, ttl, coordX, coordY, info) "+
                                                         "VALUES (?,?,?,?,?,?,?);");
            st.setInt(1, id);
            st.setBoolean(2, prio);
            st.setTimestamp(3, new Timestamp(init));
            st.setInt(4, ttl);
            st.setInt(5, coordX);
            st.setInt(6, coordY);
            st.setString(7, info);
            st.execute();
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public void insertCar(String registration, String brand, String[] modelNowner, String date){
         try {
            Statement st = conn.createStatement();
            st.execute("INSERT INTO imttCars " +
                       "VALUES ('"+registration+"','"+brand+"','"+modelNowner[0]+"','"+modelNowner[1].trim()+"','"+date+"');");
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public void printAlerts(){
        try {
            Statement st = conn.createStatement();
            ResultSet rs=st.executeQuery("SELECT * FROM imttAlerts");
            System.out.println("+------+-------+----------+-------+-------+-------+--------------+");
            System.out.println("|  ID  | Prio  |   Init   |  TTL  |   X   |   Y   |     Info     |");
            System.out.println("+------+-------+----------+-------+-------+-------+--------------+");
            while(rs.next()){
                  System.out.format("| %-4d | %-5b | %s | %-5d | %-5d | %-5d | %-12s |%n",rs.getInt(1),  rs.getBoolean(2),
                                                                                          rs.getTime(3), rs.getInt(4),
                                                                                          rs.getInt(5),  rs.getInt(6),
                                                                                          rs.getString(7));
            }
            System.out.println("+------+-------+----------+-------+-------+-------+--------------+");
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public void printAlert(Integer id){
        try {
            Statement st = conn.createStatement();
            ResultSet rs = st.executeQuery("SELECT * FROM imttAlerts WHERE id = " + id);
            System.out.println("+------+-------+----------+-------+-------+-------+--------------+");
            System.out.println("|  ID  | Prio  |   Init   |  TTL  |   X   |   Y   |     Info     |");
            System.out.println("+------+-------+----------+-------+-------+-------+--------------+");
            while(rs.next()) {
                System.out.format("| %-4d | %-5b | %s | %-5d | %-5d | %-5d | %-12s |%n",rs.getInt(1),  rs.getBoolean(2),
                                                                                      rs.getTime(3), rs.getInt(4),
                                                                                      rs.getInt(5),  rs.getInt(6),
                                                                                      rs.getString(7));
            }
            System.out.println("+------+-------+----------+-------+-------+-------+--------------+");
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public void printCars(){
        try {
            Statement st = conn.createStatement();
            ResultSet rs = st.executeQuery("SELECT * FROM imttCars");
            System.out.println("+--------------+-------+------------------------------+----------------+------------+");
            System.out.println("| Registration | Brand |             Model            |     Owner      |    Date    |");
            System.out.println("+--------------+-------+------------------------------+----------------+------------+");
            while(rs.next()){
                System.out.format("|    %-6s    | %-5s | %-28s | %-14s | %-10s |%n", rs.getString(1), rs.getString(2),
                                                                                         rs.getString(3), rs.getString(4),
                                                                                         rs.getString(5));
            }
            System.out.println("+--------------+-------+------------------------------+----------------+------------+");
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public void printCar(String registration){
        try {
            Statement st = conn.createStatement();
            ResultSet rs = st.executeQuery("SELECT * FROM imttCars WHERE registration = '" + registration + "';");
            System.out.println("+--------------+-------+------------------------------+----------------+------------+");
            System.out.println("| Registration | Brand |             Model            |     Owner      |    Date    |");
            System.out.println("+--------------+-------+------------------------------+----------------+------------+");
            while(rs.next()) {
                System.out.format("|    %-6s    | %-5s | %-28s | %-14s | %-10s |%n", rs.getString(1), rs.getString(2),
                                                                                     rs.getString(3), rs.getString(4),
                                                                                     rs.getString(5));
            }
            System.out.println("+--------------+-------+------------------------------+----------------+------------+");
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
}
