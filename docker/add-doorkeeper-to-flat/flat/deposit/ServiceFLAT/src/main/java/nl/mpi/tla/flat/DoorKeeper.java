package nl.mpi.tla.flat;

import nl.mpi.tla.flat.deposit.Flow;

import java.io.File;
import javax.servlet.ServletContext;

import javax.ws.rs.GET;
import javax.ws.rs.InternalServerErrorException;
import javax.ws.rs.NotFoundException;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.MediaType;
import net.sf.saxon.s9api.XdmValue;

@Path("doorkeeper/{sip}")
public class DoorKeeper {
    
    static private Flow flow = null;
    
    @Context
    private ServletContext servletContext;
    
    private Flow getFlow() throws Exception {
        if (flow == null)
            flow = new Flow(new File(servletContext.getInitParameter("doorkeeperConfig")));
        return flow;
    }

    @POST
    @Produces(MediaType.TEXT_PLAIN)
    public String postSip(@PathParam("sip") String sip) {
        String sipDir = "";
        try {
            XdmValue dir = this.getFlow().getContext().getProperty("work", "/tmp");
            sipDir = dir.toString()+System.getProperty("path.separator")+sip;
            File sd = new File(sipDir);
            if (!sd.isDirectory()) {
                throw new NotFoundException("The SIP["+sip+"] doesn't exist!");
            }
        } catch (Exception ex) {
            throw new InternalServerErrorException(ex.getMessage(),ex);
        }
        return "sip director["+sipDir+"]";
    }

    @GET
    @Produces(MediaType.TEXT_PLAIN)
    public String getSip(@PathParam("sip") String sip) {
        return "Got it!";
    }
    
}
