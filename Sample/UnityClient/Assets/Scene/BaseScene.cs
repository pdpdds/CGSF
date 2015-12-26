using UnityEngine;
using System.Collections;

public class BaseScene : MonoBehaviour
{
    ///씬과 관계없이 전역적으로 처리될 메세지들은 여기서 정의@
    virtual public void Awake() { }
    virtual public void Start() { }
    virtual public void Update() { }
    virtual public void Destory() { }
    //virtual public void res_Registration(NETCODE.jPacket packet) { }
    //virtual public void res_Login(NETCODE.jPacket packet) { }
    //virtual public void res_IdIsValid(NETCODE.jPacket packet) { }
    
     

}