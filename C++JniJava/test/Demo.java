package test;

/**
 * 该类是为了演示JNI如何访问各种对象属性等
 */
public class Demo {
    // 用于演示如何访问静态的基本类型属性
    public static int COUNT = 8;
    // 演示对象型属性
    private String str_data;
    private int[] counts;

    public Demo() {
        this("缺省构造函数");
    }

    /*
     * 演示如何访问构造器
     */
    public Demo(String data) {
        this.str_data = data;
        this.counts = null;
    }

    public String getStrData() {
        return str_data;
    }
    /*
     * 转换一下编码统一输出
     */
    public String setStrData(String data){
        this.str_data = data;
        return str_data;
    }

    /*
     * 该方法演示如何访问一个静态方法
     */
    public static String getHelloWorld() {
        return "Hello world!你好,世界!";
    }

    /*
     * 该方法演示参数的传入传出，类型转换及字符串的处理
     */
    public String append(String str, int value) {
        return str + value;
    }

    /*
     * 演示数组对象的访问
     */
    public int[] getCounts() {
        return counts;
    }

    /*
     * 演示如何构造一个数组对象
     */
    public void setCounts(int[] counts) {
        this.counts = counts;
    }

    /*
     * 演示异常的捕捉
     */
    public void throwExcp() throws IllegalAccessException {
        throw new IllegalAccessException("测试抛出异常.");
    }

}