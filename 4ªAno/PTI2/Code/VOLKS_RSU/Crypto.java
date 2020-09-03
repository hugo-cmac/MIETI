import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.URISyntaxException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.security.*;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.spec.X509EncodedKeySpec;
import java.util.Base64;
import java.util.HashMap;

public class Crypto {
    private PrivateKey privateKey;
    private HashMap<String, PublicKey> publicKeys;

    private static String exec(String[] command) throws InterruptedException, IOException {
        String line;
        StringBuilder result = new StringBuilder();
        Process pid = Runtime.getRuntime().exec(command);
        pid.waitFor();
        BufferedReader output = new BufferedReader(new InputStreamReader(pid.getInputStream()));
        while ((line = output.readLine()) != null) {
            if(!line.equals("-----BEGIN PRIVATE KEY-----") && !line.equals("-----END PRIVATE KEY-----"))
                result.append(line);
        }
        return result.toString();
    }

    private PrivateKey loadPrivateKey(String id, String pwd) {
        String[] command = {".././keymgmt", "-s", id, pwd};

        String skPEM = null;
        PrivateKey sk = null;
        try {
            skPEM = exec(command);
            sk = KeyFactory.getInstance("RSA").generatePrivate(
                    new PKCS8EncodedKeySpec(
                            Base64.getDecoder().decode(skPEM)
                    )
            );
        } catch (InterruptedException e)            { e.printStackTrace();}
        catch (IOException e)                       { e.printStackTrace();}
        catch (InvalidKeySpecException e)           { e.printStackTrace();}
        catch (NoSuchAlgorithmException e)          { e.printStackTrace();}
        return sk;
    }

    private PublicKey loadPublicKey(String id) {
        PublicKey pk = null;
        try {
            String pkPEM = new String(Files.readAllBytes(Paths.get("../pub/"+ id +".pem")));
            pkPEM = pkPEM.replace("-----BEGIN PUBLIC KEY-----", "").replace("-----END PUBLIC KEY-----", "").replaceAll("\\s", "");
            pk = KeyFactory.getInstance("RSA").generatePublic(new X509EncodedKeySpec(Base64.getDecoder().decode(pkPEM)));
        } catch (IOException e)                     { e.printStackTrace(); }
        catch (InvalidKeySpecException e)           { e.printStackTrace(); }
        catch (NoSuchAlgorithmException e)          { e.printStackTrace(); }
        return pk;
    }

    Crypto(String id, String pwd) {
        publicKeys = new HashMap<>();
        privateKey = loadPrivateKey(id, pwd);
        publicKeys.put("imtt", loadPublicKey("imtt"));
        publicKeys.put("vmw", loadPublicKey("vmw"));
        publicKeys.put("volks", loadPublicKey("volks"));
        publicKeys.put("car", loadPublicKey("car"));
    }

    public byte[] encrypt(byte[] pt, String id) {
        if (publicKeys.containsKey(id)) {
            Cipher cipher = null;
            byte[] ct = null;
            try {
                cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
                cipher.init(Cipher.ENCRYPT_MODE, publicKeys.get(id));
                ct = cipher.doFinal(pt);
            } catch (NoSuchAlgorithmException e)    { e.printStackTrace(); }
            catch (NoSuchPaddingException e)        { e.printStackTrace(); }
            catch (InvalidKeyException e)           { e.printStackTrace(); }
            catch (IllegalBlockSizeException e)     { e.printStackTrace(); }
            catch (BadPaddingException e)           { e.printStackTrace(); }
            return ct;
        }
        return null;
    }

    public byte[] decrypt(final byte[] ct) {
        Cipher cipher = null;
        byte[] pt = null;
        try {
            cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
            cipher.init(Cipher.DECRYPT_MODE, privateKey);
            pt = cipher.doFinal(ct);
        } catch (NoSuchAlgorithmException e)        { e.printStackTrace(); }
        catch (NoSuchPaddingException e)            { e.printStackTrace(); }
        catch (InvalidKeyException e)               { e.printStackTrace(); }
        catch (IllegalBlockSizeException e)         { e.printStackTrace(); }
        catch (BadPaddingException e)               { e.printStackTrace(); }
        return pt;
    }
}
