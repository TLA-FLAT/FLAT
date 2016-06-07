package nl.mpi.tla.flat;

import nl.mpi.tla.flat.deposit.Flow;

import java.io.File;
import java.util.HashMap;
import java.util.Map;
import javax.annotation.PostConstruct;
import javax.servlet.ServletContext;

import javax.ws.rs.GET;
import javax.ws.rs.PUT;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.core.Context;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;
import net.sf.saxon.s9api.XdmAtomicValue;
import net.sf.saxon.s9api.XdmValue;

@Path("/doorkeeper/{sip}")
public class DoorKeeperServlet {
    
    @Context
    private ServletContext servletContext;
    
    private Flow getFlow(Map<String,XdmValue> params) throws Exception {
        return new Flow(new File(servletContext.getInitParameter("doorkeeperConfig")),params);
    }

    @PUT
    @Produces("text/plain")
    public Response putSip(@PathParam("sip") String sip) {
        Map<String,XdmValue> params = new HashMap();
        params.put("sip", new XdmAtomicValue(sip));
        String sipDir = "";
        Flow flow = null;
        try {
            flow = this.getFlow(params);
            sipDir = flow.getContext().getProperty("work", "/tmp").toString();
            File sd = new File(sipDir);
            if (!sd.isDirectory()) {
                return Response.status(Status.NOT_FOUND).entity("The SIP["+sip+"] doesn't exist!").type("text/plain").build();
            }
        } catch (Exception ex) {
            return Response.status(Status.INTERNAL_SERVER_ERROR).entity(ex.getMessage()).type("text/plain").build();
        }
        DoorKeeperContextListener doorkeeperContext = (DoorKeeperContextListener)servletContext.getAttribute("DOORKEEPER");
        if (doorkeeperContext.execute(sip,flow))
            return Response.status(Status.ACCEPTED).entity("sip["+sip+"] directory["+sipDir+"]").type("text/plain").build();
        return Response.status(Status.CONFLICT).entity("ERROR: sip["+sip+"] is already being executed!").type("text/plain").build();
    }

    @GET
    @Produces("text/plain")
    public Response getSip(@PathParam("sip") String sip) {
        DoorKeeperContextListener doorkeeperContext = (DoorKeeperContextListener)servletContext.getAttribute("DOORKEEPER");
        Flow flow = doorkeeperContext.executed(sip);
        if (flow == null)
            return Response.status(Status.NOT_FOUND).entity("ERROR: The SIP["+sip+"] isn't executed!").type("text/plain").build();
        Boolean status = flow.getStatus();
        if (status == null)
            return Response.status(Status.ACCEPTED).entity("The SIP["+sip+"] is being executed!").type("text/plain").build();
        return Response.status(Status.OK).entity("The SIP["+sip+"] has been executed"+(status.booleanValue()?" succesfully!":", but failed!")).type("text/plain").build();
    }
    
}
